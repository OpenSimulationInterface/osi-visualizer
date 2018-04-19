#include "osiparser.h"


#include <QFileDialog>
#include <QMessageBox>

/*
 *
 * This function is called when a new zmq message is received
 * and successfully parsed into SensorData.
 * According to the desired data type, the received osi message will be parsed
 * and a signal with parsed data (objects and lanes) will be emitted.
 *
 */

OsiParser::OsiParser(const int osiMsgSaveThreshold)
    : isFirstMessage_(true)
    , startSaveOSIMsg_(false)
    , osiMsgString_("")
    , osiMsgNumber_(0)
    , osiMsgSaveThreshold_(osiMsgSaveThreshold)
{

}

void
OsiParser::CancelSaveOSIMessage()
{
    isFirstMessage_ = true;
    startSaveOSIMsg_ = false;
    osiMsgNumber_ = 0;
    osiMsgString_.clear();
    emit EnableExport(false);
}

void
OsiParser::ParseReceivedMessage(const osi3::SensorData& sensorData,
                                const DataType datatype)
{
    if (isFirstMessage_)
    {
        isFirstMessage_ = false;
        emit EnableExport(true);
    }
    else if(startSaveOSIMsg_ && osiMsgNumber_ < osiMsgSaveThreshold_)
    {
        ++osiMsgNumber_;

        osiMsgString_ += sensorData.SerializeAsString();
        osiMsgString_ += "$$__$$";
    }
    else if(osiMsgNumber_ >= osiMsgSaveThreshold_)
    {
        emit SaveOSIMsgOverflow(osiMsgSaveThreshold_);
    }

    Message objectMessage;
    LaneMessage laneMessage;

    if (datatype == DataType::Groundtruth)
    {
        const osi3::GroundTruth& groundTruth = sensorData.sensor_view(0).global_ground_truth();
        ParseGroundtruth(groundTruth, objectMessage, laneMessage);
    }
    else if (datatype == DataType::SensorData)
    {
        ParseSensorData(sensorData, objectMessage, laneMessage);
    }

    emit MessageParsed(objectMessage, laneMessage);
}

//Parse Ground Truth Data
void
OsiParser::ParseGroundtruth(const osi3::GroundTruth& groundTruth,
                            Message& objectMessage,
                            LaneMessage& laneMessage)
{

    for (int i = 0; i < groundTruth.moving_object_size(); ++i)
    {
        const osi3::MovingObject& object = groundTruth.moving_object(i);
        ObjectType objectType = GetObjectTypeFromOsiObjectType(object.type());
        QString idStr = QString::number(object.id().value());

        ParseGroundtruthMovingObject(objectMessage,
                                     object.base(),
                                     objectType,
                                     idStr);
    }

    for (int i = 0; i < groundTruth.stationary_object_size(); ++i)
    {
        const osi3::StationaryObject& object = groundTruth.stationary_object(i);
        ObjectType objectType = GetObjectTypeFromOsiObjectType(object.classification().type());
        QString idStr = QString::number(object.id().value());

        ParseGroundtruthStationaryObject(objectMessage,
                                         object.base(),
                                         objectType,
                                         idStr);
    }

    for (int i = 0; i < groundTruth.moving_object_size(); ++i)
    {
        const osi3::MovingObject& mvObj = groundTruth.moving_object(i);
        ObjectType objectType = GetObjectTypeFromOsiObjectType(mvObj.type());
        QString idStr = QString::number(mvObj.id().value());

        ParseGroundtruthMovingObject(objectMessage,
                                     mvObj.base(),
                                     objectType,
                                     idStr);
    }

    for (int i = 0; i < groundTruth.traffic_sign_size(); ++i)
    {
        const osi3::TrafficSign& sign = groundTruth.traffic_sign(i);
        QString idStr = QString::number(sign.id().value());

        ParseGroundtruthStationaryObject(objectMessage,
                                         sign.main_sign().base(),
                                         ObjectType::TrafficSign,
                                         idStr);
    }


    for (int i = 0; i < groundTruth.traffic_light_size(); ++i)
    {
        const osi3::TrafficLight& light = groundTruth.traffic_light(i);
        QString idStr = QString::number(light.id().value());

        ParseGroundtruthStationaryObject(objectMessage,
                                         light.base(),
                                         ObjectType::TrafficLight,
                                         idStr);
    }

    // DRAW_CENTER_LINES
    for (int i = 0; i < groundTruth.lane_size(); ++i)
    {
        const osi3::Lane& lane = groundTruth.lane(i);
        LaneStruct tmpLane;
        tmpLane.id = lane.id().value();

        QVector<QVector3D> centerLines;
        for (int a = 0; a < lane.classification().centerline_size(); ++a)
        {
            const osi3::Vector3d& centerLine = lane.classification().centerline(a);
            centerLines.append(QVector3D(centerLine.y(), 0, centerLine.x()));
        }

        tmpLane.centerLanes.append(centerLines);

        laneMessage.append(tmpLane);
    }

    // DRAW_LANE_BOUNDARIES
    for (int b = 0; b < groundTruth.lane_boundary_size(); ++b)
    {
        const osi3::LaneBoundary& laneBoundary = groundTruth.lane_boundary(b);
        LaneStruct tmpLane;
        tmpLane.id = laneBoundary.id().value();

        QVector<QVector3D> boundaryLines;
        for (int c = 0; c < laneBoundary.boundary_line_size(); ++c)
        {
            const osi3::LaneBoundary_BoundaryPoint& boundaryPoint = laneBoundary.boundary_line(c);
            const osi3::Vector3d& position = boundaryPoint.position();
            boundaryLines.append(QVector3D(position.y(), 0, position.x()));
        }

        tmpLane.boundaryLanes.append(boundaryLines);

        laneMessage.append(tmpLane);
    }
}


// Parse a moving ground truth object
void
OsiParser::ParseGroundtruthMovingObject(Message& objectMessage,
                                        const osi3::BaseMoving& baseObject,
                                        const ObjectType objectType,
                                        const QString& idStr)
{
    if (objectType == ObjectType::None)
    {
        return;
    }

    osi3::Vector3d position = baseObject.position();
    osi3::Vector3d velocity = baseObject.velocity();
    osi3::Vector3d acceleration = baseObject.acceleration();
    float orientation = baseObject.orientation().yaw();
    osi3::Dimension3d dimension = baseObject.dimension();

    if (dimension.width() == 0 && dimension.length() == 0)
    {
        dimension.set_width(1.0f);
        dimension.set_length(1.0f);
    }

    MessageStruct tmpMsg;
    tmpMsg.id = Global::GetObjectTypeName(objectType) + idStr;
    tmpMsg.name = "ID: " + idStr;
    tmpMsg.type = objectType;
    tmpMsg.orientation = orientation+M_PI_2;
    tmpMsg.position = QVector3D(position.y(), 0, position.x());
    tmpMsg.realPosition = QVector3D(position.x(), position.y(), position.z());
    tmpMsg.velocitie = QVector3D(velocity.x(), velocity.y(), velocity.z());
    tmpMsg.acceleration = QVector3D(acceleration.x(), acceleration.y(), acceleration.z());
    tmpMsg.dimension = dimension;

    objectMessage.append(tmpMsg);
}


// Parse a stationary ground truth object
void
OsiParser::ParseGroundtruthStationaryObject(Message& objectMessage,
                                            const osi3::BaseStationary& baseObject,
                                            const ObjectType objectType,
                                            const QString& idStr)
{
    if (objectType == ObjectType::None)
    {
        return;
    }

    osi3::Vector3d tmp;
    tmp.set_x(0.0);
    tmp.set_y(0.0);
    tmp.set_z(0.0);

    osi3::Vector3d position = baseObject.position();
    osi3::Vector3d velocity = tmp;
    osi3::Vector3d acceleration = tmp;
    float orientation = baseObject.orientation().yaw();
    osi3::Dimension3d dimension = baseObject.dimension();

    if (dimension.width() == 0 && dimension.length() == 0)
    {
        dimension.set_width(1.0f);
        dimension.set_length(1.0f);
    }

    MessageStruct tmpMsg;
    tmpMsg.id = Global::GetObjectTypeName(objectType) + idStr;
    tmpMsg.name = "ID: " + idStr;
    tmpMsg.type = objectType;
    tmpMsg.orientation = orientation+M_PI_2;
    tmpMsg.position = QVector3D(position.y(), 0, position.x());
    tmpMsg.realPosition = QVector3D(position.x(), position.y(), position.z());
    tmpMsg.velocitie = QVector3D(velocity.x(), velocity.y(), velocity.z());
    tmpMsg.acceleration = QVector3D(acceleration.x(), acceleration.y(), acceleration.z());
    tmpMsg.dimension = dimension;

    objectMessage.append(tmpMsg);
}


// Parse SensorData
void
OsiParser::ParseSensorData(const osi3::SensorData &sensorData,
                           Message& objectMessage,
                           LaneMessage& laneMessage)
{
    for (int i = 0; i < sensorData.moving_object_size(); ++i)
    {
        const osi3::DetectedMovingObject& dectObj = sensorData.moving_object(i);
        double highestProbability = -1;
        int highestIndex = 0;
        for(int j = 0; j < dectObj.candidate().size(); ++j)
        {
            const osi3::DetectedMovingObject_CandidateMovingObject candi = dectObj.candidate(j);
            if(candi.probability() > highestProbability)
            {
                highestProbability = candi.probability();
                highestIndex = j;
            }
        }

        ObjectType objectType = GetObjectTypeFromOsiObjectType(dectObj.candidate(highestIndex).vehicle_classification().type());
        QString idStr = QString::number(dectObj.header().ground_truth_id(highestIndex).value());
        ParseSensorDataMovingObject(objectMessage, dectObj.base(), objectType, idStr);
    }

    for (int i = 0; i < sensorData.traffic_sign_size(); ++i)
    {
        const osi3::DetectedTrafficSign_DetectedMainSign& dectMS = sensorData.traffic_sign(i).main_sign();
        QString idStr = QString::number(sensorData.traffic_sign(i).header().tracking_id().value());
        ParseSensorDataStationaryObject(objectMessage, dectMS.base(), ObjectType::TrafficSign, idStr);
    }

    // DRAW_CENTER_LINES
    for (int i = 0; i < sensorData.lane_size(); ++i)
    {
        const osi3::DetectedLane& detectL = sensorData.lane(i);

        int highestProbability = -1;
        int highestProbabilityIndex = -1;
        for(int i = 0; i < detectL.candidate_size(); ++i)
        {
            if(highestProbability < detectL.candidate(i).probability())
            {
                highestProbability = detectL.candidate(i).probability();
                highestProbabilityIndex = i;
            }
        }

        LaneStruct tmpLane;
        tmpLane.id = detectL.header().tracking_id().value();

        QVector<QVector3D> centerLines;
        const osi3::Lane_Classification& laneC = detectL.candidate(highestProbabilityIndex).classification();

        for(int c = 0; c < laneC.centerline_size(); ++c)
        {
            const osi3::Vector3d& centerLine = laneC.centerline(c);
            if(centerLine.x() != 0 || centerLine.y() != 0 || centerLine.z() != 0)
            {
                centerLines.append(QVector3D(centerLine.x(), 0, -centerLine.y()));
            }
            else if(!centerLines.empty())
            {
                tmpLane.centerLanes.append(centerLines);
                centerLines.clear();
            }
        }
        if(!centerLines.empty())
            tmpLane.centerLanes.append(centerLines);

        if(!tmpLane.centerLanes.empty())
            laneMessage.append(tmpLane);
    }

    // DRAW_LANE_BOUNDARIES
    for (int i = 0; i < sensorData.lane_boundary_size(); ++i)
    {
        const osi3::DetectedLaneBoundary& dLaneB = sensorData.lane_boundary(i);

        LaneStruct tmpLane;
        tmpLane.id = dLaneB.header().tracking_id().value();

        QVector<QVector3D> boundaryLines;

        for(int b = 0; b < dLaneB.boundary_line_size(); ++b)
        {
            const osi3::Vector3d& position = dLaneB.boundary_line(b).position();

            if(position.x() != 0 || position.y() != 0 || position.z() != 0)
            {
                boundaryLines.append(QVector3D(position.y(), 0, position.x()));
            }
            else if(!boundaryLines.empty())
            {
                tmpLane.boundaryLanes.append(boundaryLines);
                boundaryLines.clear();
            }
        }

        if(!boundaryLines.empty())
            tmpLane.boundaryLanes.append(boundaryLines);

        if(!tmpLane.boundaryLanes.empty())
            laneMessage.append(tmpLane);
    }
}

/*
 *
 * Parse a moving SensorData Object
 *
 */
void
OsiParser::ParseSensorDataMovingObject(Message& objectMessage,
                                       const osi3::BaseMoving& baseObject,
                                       const ObjectType objectType,
                                       const QString& idStr)
{
    if (objectType == ObjectType::None)
    {
        return;
    }

    osi3::Vector3d position = baseObject.position();
    osi3::Vector3d velocity = baseObject.velocity();
    osi3::Vector3d acceleration = baseObject.acceleration();
    float orientation = baseObject.orientation().yaw();

    MessageStruct tmpMsg;
    tmpMsg.id = Global::GetObjectTypeName(objectType) + idStr;
    tmpMsg.name = "ID: " + idStr;
    tmpMsg.type = objectType;
    tmpMsg.orientation = orientation;
    tmpMsg.position = QVector3D(position.x(), 0, -position.y());
    tmpMsg.realPosition = QVector3D(position.x(), position.y(), position.z());
    tmpMsg.velocitie = QVector3D(velocity.x(), velocity.y(), velocity.z());
    tmpMsg.acceleration = QVector3D(acceleration.x(), acceleration.y(), acceleration.z());

    if(baseObject.base_polygon_size() == 4)
    {
        for(int i = 0; i < 4; ++i)
        {
            const osi3::Vector2d& vertice = baseObject.base_polygon(i);
            tmpMsg.basePoly.push_back(QVector3D(vertice.x() - position.x(), 0, -vertice.y()+position.y()));
        }
    }
    else
    {
        osi3::Dimension3d dimension = baseObject.dimension();
        if (dimension.width() == 0 && dimension.length() == 0)
        {
            dimension.set_width(1.0f);
            dimension.set_length(1.0f);
        }
        tmpMsg.dimension = dimension;
    }

    objectMessage.append(tmpMsg);
}

void
OsiParser::ParseSensorDataStationaryObject(Message& objectMessage,
                                           const osi3::BaseStationary& baseObject,
                                           const ObjectType objectType,
                                           const QString& idStr)
{
    if (objectType == ObjectType::None)
    {
        return;
    }

    osi3::Vector3d tmp;
    tmp.set_x(0.0);
    tmp.set_y(0.0);
    tmp.set_z(0.0);

    osi3::Vector3d position = baseObject.position();
    osi3::Vector3d velocity = tmp;
    osi3::Vector3d acceleration = tmp;
    float orientation = baseObject.orientation().yaw();

    MessageStruct tmpMsg;
    tmpMsg.id = Global::GetObjectTypeName(objectType) + idStr;
    tmpMsg.name = "ID: " + idStr;
    tmpMsg.type = objectType;
    tmpMsg.orientation = orientation;
    tmpMsg.position = QVector3D(position.x(), 0, -position.y());
    tmpMsg.realPosition = QVector3D(position.x(), position.y(), position.z());
    tmpMsg.velocitie = QVector3D(velocity.x(), velocity.y(), velocity.z());
    tmpMsg.acceleration = QVector3D(acceleration.x(), acceleration.y(), acceleration.z());

    if(baseObject.base_polygon_size() == 4)
    {
        for(int i = 0; i < 4; ++i)
        {
            const osi3::Vector2d& vertice = baseObject.base_polygon(i);
            tmpMsg.basePoly.push_back(QVector3D(vertice.x(), 0, -vertice.y()));
        }
    }
    else
    {
        osi3::Dimension3d dimension = baseObject.dimension();
        if (dimension.width() == 0 && dimension.length() == 0)
        {
            dimension.set_width(1.0f);
            dimension.set_length(1.0f);
        }
        tmpMsg.dimension = dimension;
    }

    objectMessage.append(tmpMsg);
}

//Export Osi message
void
OsiParser::ExportOsiMessage()
{
    if(startSaveOSIMsg_ == false && osiMsgString_.empty() == true)
    {
        startSaveOSIMsg_ = true;
    }
    else if(startSaveOSIMsg_ == true && osiMsgString_.empty() == false)
    {
        startSaveOSIMsg_ = false;

        QString fileName = QFileDialog::getSaveFileName(nullptr, "Export message to file", "", "*.txt");
        if (fileName.isEmpty() == false)
        {
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly))
            {
                qDebug() << "Error opening file '" + fileName + "' for writing.";
                return;
            }

            file.write(osiMsgString_.c_str(), osiMsgString_.length());
            file.close();
        }

        osiMsgNumber_ = 0;
        osiMsgString_.clear();
    }
    else
    {
        // should never come here
        QMessageBox::warning(nullptr, "Warning", "Nothing to save!", QMessageBox::Ok, QMessageBox::Ok);
    }
}

// Convert object types
ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi3::MovingObject_VehicleClassification_Type& vehicleType)
{
    ObjectType objType = ObjectType::None;
    switch (vehicleType) {
        case osi3::MovingObject_VehicleClassification_Type_TYPE_UNKNOWN:
            objType = ObjectType::UnknownObject;
            break;
        case osi3::MovingObject_VehicleClassification_Type_TYPE_OTHER:
            objType = ObjectType::OtherObject;
            break;
        case osi3::MovingObject_VehicleClassification_Type_TYPE_SMALL_CAR:
        case osi3::MovingObject_VehicleClassification_Type_TYPE_COMPACT_CAR:
        case osi3::MovingObject_VehicleClassification_Type_TYPE_MEDIUM_CAR:
        case osi3::MovingObject_VehicleClassification_Type_TYPE_LUXURY_CAR:
            objType = ObjectType::Car;
            break;
        case osi3::MovingObject_VehicleClassification_Type_TYPE_DELIVERY_VAN:
        case osi3::MovingObject_VehicleClassification_Type_TYPE_HEAVY_TRUCK:
            objType = ObjectType::Truck;
            break;
        case osi3::MovingObject_VehicleClassification_Type_TYPE_MOTORBIKE:
            objType = ObjectType::MotorBike;
            break;
        case osi3::MovingObject_VehicleClassification_Type_TYPE_BICYCLE:
            objType = ObjectType::Bicycle;
            break;
        case osi3::MovingObject_VehicleClassification_Type_TYPE_SEMITRAILER:
        case osi3::MovingObject_VehicleClassification_Type_TYPE_TRAILER:
            objType = ObjectType::Trailer;
            break;
        default:
            break;
    }

    return objType;
}

ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi3::MovingObject_Type& objectType)
{
    ObjectType objType = ObjectType::None;
    switch (objectType) {
        case osi3::MovingObject_Type_TYPE_UNKNOWN:
            objType = ObjectType::UnknownObject;
            break;
        case osi3::MovingObject_Type_TYPE_OTHER:
            objType = ObjectType::OtherObject;
            break;
        case osi3::MovingObject_Type_TYPE_VEHICLE:
            objType = ObjectType::Pedestrian;
            break;
        case osi3::MovingObject_Type_TYPE_PEDESTRIAN:
            objType = ObjectType::Pedestrian;
            break;
        case osi3::MovingObject_Type_TYPE_ANIMAL:
            objType = ObjectType::Animal;
            break;
        default:
            break;
    }

    return objType;
}

ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi3::StationaryObject_Classification_Type& objectType)
{
    ObjectType objType = ObjectType::None;
    switch (objectType) {
        case osi3::StationaryObject_Classification_Type_TYPE_UNKNOWN:
            objType = ObjectType::UnknownObject;
            break;
        case osi3::StationaryObject_Classification_Type_TYPE_OTHER:
            objType = ObjectType::OtherObject;
            break;
        case osi3::StationaryObject_Classification_Type_TYPE_BRIDGE:
            objType = ObjectType::Bridge;
            break;
        case osi3::StationaryObject_Classification_Type_TYPE_BUILDING:
            objType = ObjectType::Building;
            break;
        case osi3::StationaryObject_Classification_Type_TYPE_PYLON:
            objType = ObjectType::Pylon;
            break;
        case osi3::StationaryObject_Classification_Type_TYPE_REFLECTIVE_STRUCTURE:
            objType = ObjectType::ReflectorPost;
            break;
        case osi3::StationaryObject_Classification_Type_TYPE_DELINEATOR:
            objType = ObjectType::Delineator;
            break;
        default:
            break;
    }

    return objType;
}

ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi3::TrafficSign_MainSign_Classification_Type& objectType)
{
    return ObjectType::TrafficSign;
}


ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi3::TrafficLight_Classification_Mode& objectType)
{
    return ObjectType::TrafficLight;
}



