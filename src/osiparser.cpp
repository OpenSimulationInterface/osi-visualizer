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
    : currentGroundTruth_()
    , currentSensorData_()
    , currentDataType_()
    , isFirstMessage_(true)
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
OsiParser::ParseReceivedMessage(const osi::SensorData& sensorData,
                                const DataType datatype)
{
    currentSensorData_ = sensorData;
    currentDataType_ = datatype;

    if (isFirstMessage_)
    {
        isFirstMessage_ = false;
        emit EnableExport(true);
    }
    else if(startSaveOSIMsg_ && osiMsgNumber_ < osiMsgSaveThreshold_)
    {
        ++osiMsgNumber_;

        osiMsgString_ += currentSensorData_.SerializeAsString();
        osiMsgString_ += "$$__$$";
    }
    else if(osiMsgNumber_ >= osiMsgSaveThreshold_)
    {
        emit SaveOSIMsgOverflow(osiMsgSaveThreshold_);
    }

    Message objectMessage;
    LaneMessage laneMessage;

    if (currentDataType_ == DataType::Groundtruth)
    {
        currentGroundTruth_ = *(currentSensorData_.mutable_ground_truth()->mutable_global_ground_truth());
        ParseGroundtruth(objectMessage, laneMessage);
    }
    else if (currentDataType_ == DataType::SensorData)
    {
        ParseSensorData(objectMessage, laneMessage);
    }

    emit MessageParsed(objectMessage, laneMessage);
}

//Parse Ground Truth Data
void
OsiParser::ParseGroundtruth(Message& objectMessage,
                            LaneMessage& laneMessage)
{

    for (int i = 0; i < currentGroundTruth_.moving_object_size(); ++i)
    {
        osi::MovingObject object = currentGroundTruth_.moving_object(i);
        ObjectType objectType = GetObjectTypeFromOsiObjectType(object.type());
        QString idStr = QString::number(object.id().value());

        ParseGroundtruthMovingObject(objectMessage,
                                     object.base(),
                                     objectType,
                                     idStr,
                                     false);
    }

    for (int i = 0; i < currentGroundTruth_.stationary_object_size(); ++i)
    {
        osi::StationaryObject object = currentGroundTruth_.stationary_object(i);
        ObjectType objectType = GetObjectTypeFromOsiObjectType(object.type());
        QString idStr = QString::number(object.id().value());

        ParseGroundtruthStationaryObject(objectMessage,
                                         object.base(),
                                         objectType,
                                         idStr,
                                         false);
    }

    for (int i = 0; i < currentGroundTruth_.vehicle_size(); ++i)
    {
        osi::Vehicle vehicle = currentGroundTruth_.vehicle(i);
        ObjectType objectType = GetObjectTypeFromOsiVehicleType(vehicle.type());
        QString idStr = QString::number(vehicle.id().value());

        ParseGroundtruthMovingObject(objectMessage,
                                     vehicle.base(),
                                     objectType,
                                     idStr,
                                     vehicle.ego_vehicle());
    }

    for (int i = 0; i < currentGroundTruth_.traffic_sign_size(); ++i)
    {
        osi::TrafficSign sign = currentGroundTruth_.traffic_sign(i);
        QString idStr = QString::number(sign.id().value());

        ParseGroundtruthStationaryObject(objectMessage,
                                         sign.base(),
                                         ObjectType::TrafficSign,
                                         idStr,
                                         false);
    }


    for (int i = 0; i < currentGroundTruth_.traffic_light_size(); ++i)
    {
        osi::TrafficLight light = currentGroundTruth_.traffic_light(i);
        QString idStr = QString::number(light.id().value());

        ParseGroundtruthStationaryObject(objectMessage,
                                         light.base(),
                                         ObjectType::TrafficLight,
                                         idStr,
                                         false);
    }

    for (int i = 0; i < currentGroundTruth_.lane_size(); ++i)
    {
        osi::Lane lane = currentGroundTruth_.lane(i);
        LaneStruct tmpLane;
        tmpLane.id = lane.id().value();

        // DRAW_CENTER_LINES
        {
            // lane <dummyVector <centerLines <position> > >
            QVector<QVector3D> centerLines;
            for (int a = 0; a < lane.center_line_size(); ++a)
            {
                osi::Vector3d centerLine = lane.center_line(a);
                centerLines.append(QVector3D(-centerLine.x(), 0, centerLine.y()));
            }

            tmpLane.centerLanes.append(centerLines);
        }

        // DRAW_LANE_BOUNDARIES
        {
            // lanes <laneBoundaries <boundaryLines <position> > >
            QVector<QVector<QVector3D>> laneBoundaries;

            for (int b = 0; b < lane.lane_boundary_size(); ++b)
            {
                osi::LaneBoundary laneBoundary = lane.lane_boundary(b);
                QVector<QVector3D> boundaryLines;

                for (int c = 0; c < laneBoundary.boundary_line_size(); ++c)
                {
                    osi::BoundaryPoint boundaryLine = laneBoundary.boundary_line(c);
                    osi::Vector3d position = boundaryLine.position();
                    boundaryLines.append(QVector3D(-position.x(), 0, position.y()));
                }

                laneBoundaries.append(boundaryLines);
            }

            tmpLane.boundaryLanes= laneBoundaries;
        }

        laneMessage.append(tmpLane);
    }
}


// Parse a moving ground truth object
void
OsiParser::ParseGroundtruthMovingObject(Message& objectMessage,
                                        const osi::BaseMoving& baseObject,
                                        const ObjectType objectType,
                                        const QString& idStr,
                                        const bool isEgoVehicle)
{
    if (objectType == ObjectType::None)
    {
        return;
    }

    osi::Vector3d position = baseObject.position();
    osi::Vector3d velocity = baseObject.velocity();
    osi::Vector3d acceleration = baseObject.acceleration();
    float orientation = baseObject.orientation().yaw();
    osi::Dimension3d dimension = baseObject.dimension();

    if (dimension.width() == 0 && dimension.length() == 0)
    {
        dimension.set_width(1.0f);
        dimension.set_length(1.0f);
    }

    MessageStruct tmpMsg;
    tmpMsg.id = Global::GetObjectTypeName(objectType) + idStr;
    tmpMsg.name = "ID: " + idStr;
    tmpMsg.type = objectType;
    tmpMsg.isEgoVehicle = isEgoVehicle;
    tmpMsg.orientation = orientation;
    tmpMsg.position = QVector3D(-position.x(), 0, position.y());
    tmpMsg.realPosition = QVector3D(position.x(), position.y(), position.z());
    tmpMsg.velocitie = QVector3D(velocity.x(), velocity.y(), velocity.z());
    tmpMsg.acceleration = QVector3D(acceleration.x(), acceleration.y(), acceleration.z());
    tmpMsg.dimension = dimension;

    objectMessage.append(tmpMsg);
}


// Parse a stationary ground truth object
void
OsiParser::ParseGroundtruthStationaryObject(Message& objectMessage,
                                            const osi::BaseStationary& baseObject,
                                            const ObjectType objectType,
                                            const QString& idStr,
                                            const bool isEgoVehicle)
{
    if (objectType == ObjectType::None)
    {
        return;
    }

    osi::Vector3d tmp;
    tmp.set_x(0.0);
    tmp.set_y(0.0);
    tmp.set_z(0.0);

    osi::Vector3d position = baseObject.position();
    osi::Vector3d velocity = tmp;
    osi::Vector3d acceleration = tmp;
    float orientation = baseObject.orientation().yaw();
    osi::Dimension3d dimension = baseObject.dimension();

    if (dimension.width() == 0 && dimension.length() == 0)
    {
        dimension.set_width(1.0f);
        dimension.set_length(1.0f);
    }

    MessageStruct tmpMsg;
    tmpMsg.id = Global::GetObjectTypeName(objectType) + idStr;
    tmpMsg.name = "ID: " + idStr;
    tmpMsg.type = objectType;
    tmpMsg.isEgoVehicle = isEgoVehicle;
    tmpMsg.orientation = orientation;
    tmpMsg.position = QVector3D(-position.x(), 0, position.y());
    tmpMsg.realPosition = QVector3D(position.x(), position.y(), position.z());
    tmpMsg.velocitie = QVector3D(velocity.x(), velocity.y(), velocity.z());
    tmpMsg.acceleration = QVector3D(acceleration.x(), acceleration.y(), acceleration.z());
    tmpMsg.dimension = dimension;

    objectMessage.append(tmpMsg);
}


// Parse SensorData
void
OsiParser::ParseSensorData(Message& objectMessage,
                           LaneMessage& laneMessage)
{
    for (int i = 0; i < currentSensorData_.object_size(); ++i)
    {
        osi::DetectedObject sensorDataObject = currentSensorData_.object(i);
        if(sensorDataObject.model_internal_object().is_seen())
        {

            ObjectType objectType = GetObjectTypeFromHighestProbability(sensorDataObject.class_probability());

            if (objectType == ObjectType::None)
            {
                continue;
            }

            QString idStr = QString::number(sensorDataObject.ground_truth_id(0).value());

            ParseSensorDataMovingObject(objectMessage, sensorDataObject.object(), objectType, idStr);
        }
    }

//    for (int i = 0; i < currentSensorData_.traffic_sign_size(); ++i)
//    {
//        osi::DetectedTrafficSign sensorDataSign = currentSensorData_.traffic_sign(i);
//        osi::TrafficSign signToDisplay;
//
//        double highestProbCandidate = -1.0f;
//        int signToDisplayID;
//
//        for(int j = 0; j < sensorDataSign.candidate_sign_size(); ++j)
//        {
//
//            if(sensorDataSign.candidate_sign(j).candidate_probability() > highestProbCandidate)
//            {
//                signToDisplayID = j;
//            }
//        }
//
//        signToDisplay = sensorDataSign.mutable_candidate_sign(signToDisplayID)->sign();
//
//        QString idStr = QString::number(signToDisplay.id().value());
//
//        ParseSensorDataStationaryObject(objectMessage, signToDisplay.base(), ObjectType::TrafficSign, idStr);
//    }

    for (int i = 0; i < currentSensorData_.lane_size(); ++i)
    {
        osi::DetectedLane lane = currentSensorData_.lane(i);
        if(lane.existence_probability() == 1)
        {
            LaneStruct tmpLane;
            tmpLane.id = lane.lane().id().value();

            // DRAW_CENTER_LINES
            {
                // lane <dummyVector <centerLines <position> > >
                QVector<QVector3D> centerLines;
                for (int a = 0; a < lane.lane().center_line_size(); ++a)
                {
                    osi::Vector3d centerLine = lane.lane().center_line(a);
                    if(centerLine.x() != 0 || centerLine.y() != 0 || centerLine.z() != 0)
                    {
                        centerLines.append(QVector3D(centerLine.x(), 0, -centerLine.y()));
                    }
                    else
                    {
                        tmpLane.centerLanes.append(centerLines);
                        centerLine.Clear();
                    }
                }

                tmpLane.centerLanes.append(centerLines);
            }

            // DRAW_LANE_BOUNDARIES
            {
                // lanes <laneBoundaries <boundaryLines <position> > >
                QVector<QVector<QVector3D>> laneBoundaries;

                for (int b = 0; b < lane.lane().lane_boundary_size(); ++b)
                {
                    osi::LaneBoundary laneBoundary = lane.lane().lane_boundary(b);
                    QVector<QVector3D> boundaryLines;

                    for (int c = 0; c < laneBoundary.boundary_line_size(); ++c)
                    {
                        osi::BoundaryPoint boundaryLine = laneBoundary.boundary_line(c);
                        osi::Vector3d position = boundaryLine.position();

                        if(position.x() != 0 || position.y() != 0 || position.z() != 0)
                        {
                            boundaryLines.append(QVector3D(position.x(), 0, -position.y()));
                        }
                        else
                        {
                            laneBoundaries.append(boundaryLines);
                            boundaryLines.clear();
                        }
                    }

                    laneBoundaries.append(boundaryLines);
                }

                tmpLane.boundaryLanes= laneBoundaries;
            }

            laneMessage.append(tmpLane);
        }
    }
}


/*
 *
 * Parse a moving SensorData Object
 *
 */

void
OsiParser::ParseSensorDataMovingObject(Message& objectMessage,
                                       const osi::BaseMoving& baseObject,
                                       const ObjectType objectType,
                                       const QString& idStr)
{
    if (objectType == ObjectType::None)
    {
        return;
    }

    osi::Vector3d position = baseObject.position();
    osi::Vector3d velocity = baseObject.velocity();
    osi::Vector3d acceleration = baseObject.acceleration();
    float orientation = baseObject.orientation().yaw();
    osi::Dimension3d dimension = baseObject.dimension();

    if (dimension.width() == 0 && dimension.length() == 0)
    {
        dimension.set_width(1.0f);
        dimension.set_length(1.0f);
    }

    MessageStruct tmpMsg;
    tmpMsg.id = Global::GetObjectTypeName(objectType) + idStr;
    tmpMsg.name = "ID: " + idStr;
    tmpMsg.type = objectType;
    tmpMsg.isEgoVehicle = false;
    tmpMsg.orientation = orientation;
    tmpMsg.position = QVector3D(position.x(), 0, -position.y());
    tmpMsg.realPosition = QVector3D(position.x(), position.y(), position.z());
    tmpMsg.velocitie = QVector3D(velocity.x(), velocity.y(), velocity.z());
    tmpMsg.acceleration = QVector3D(acceleration.x(), acceleration.y(), acceleration.z());
    tmpMsg.dimension = dimension;

    objectMessage.append(tmpMsg);
}

void
OsiParser::ParseSensorDataStationaryObject(Message& objectMessage,
                                           const osi::BaseStationary& baseObject,
                                           const ObjectType objectType,
                                           const QString& idStr)
{
    if (objectType == ObjectType::None)
    {
        return;
    }

    osi::Vector3d tmp;
    tmp.set_x(0.0);
    tmp.set_y(0.0);
    tmp.set_z(0.0);

    osi::Vector3d position = baseObject.position();
    osi::Vector3d velocity = tmp;
    osi::Vector3d acceleration = tmp;
    float orientation = baseObject.orientation().yaw();
    osi::Dimension3d dimension = baseObject.dimension();

    if (dimension.width() == 0 && dimension.length() == 0)
    {
        dimension.set_width(1.0f);
        dimension.set_length(1.0f);
    }

    MessageStruct tmpMsg;
    tmpMsg.id = Global::GetObjectTypeName(objectType) + idStr;
    tmpMsg.name = "ID: " + idStr;
    tmpMsg.type = objectType;
    tmpMsg.isEgoVehicle = false;
    tmpMsg.orientation = orientation;
    tmpMsg.position = QVector3D(position.x(), 0, -position.y());
    tmpMsg.realPosition = QVector3D(position.x(), position.y(), position.z());
    tmpMsg.velocitie = QVector3D(velocity.x(), velocity.y(), velocity.z());
    tmpMsg.acceleration = QVector3D(acceleration.x(), acceleration.y(), acceleration.z());
    tmpMsg.dimension = dimension;

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
OsiParser::GetObjectTypeFromOsiVehicleType(const osi::Vehicle_Type vehicleType)
{
    ObjectType objType = ObjectType::None;
    switch (vehicleType) {
        case osi::Vehicle_Type_TYPE_UNKNOWN:
            objType = ObjectType::UnknownObject;
            break;
        case osi::Vehicle_Type_TYPE_OTHER:
            objType = ObjectType::OtherObject;
            break;
        case osi::Vehicle_Type_TYPE_CAR:
            objType = ObjectType::Car;
            break;
        case osi::Vehicle_Type_TYPE_TRUCK:
            objType = ObjectType::Truck;
            break;
        case osi::Vehicle_Type_TYPE_MOTOR_BIKE:
            objType = ObjectType::MotorBike;
            break;
        case osi::Vehicle_Type_TYPE_BICYCLE:
            objType = ObjectType::Bicycle;
            break;
        case osi::Vehicle_Type_TYPE_TRAILER:
            objType = ObjectType::Trailer;
            break;
        default:
            break;
    }

    return objType;
}

ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi::MovingObject_Type objectType)
{
    ObjectType objType = ObjectType::None;
    switch (objectType) {
        case osi::MovingObject_Type_TYPE_UNKNOWN:
            objType = ObjectType::UnknownObject;
            break;
        case osi::MovingObject_Type_TYPE_OTHER:
            objType = ObjectType::OtherObject;
            break;
        case osi::MovingObject_Type_TYPE_PEDESTRIAN:
            objType = ObjectType::Pedestrian;
            break;
        case osi::MovingObject_Type_TYPE_ANIMAL:
            objType = ObjectType::Animal;
            break;
        default:
            break;
    }

    return objType;
}

ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi::StationaryObject_Type objectType)
{
    ObjectType objType = ObjectType::None;
    switch (objectType) {
        case osi::StationaryObject_Type_TYPE_UNKNOWN:
            objType = ObjectType::UnknownObject;
            break;
        case osi::StationaryObject_Type_TYPE_OTHER:
            objType = ObjectType::OtherObject;
            break;
        case osi::StationaryObject_Type_TYPE_BRIDGE:
            objType = ObjectType::Bridge;
            break;
        case osi::StationaryObject_Type_TYPE_BUILDING:
            objType = ObjectType::Building;
            break;
        case osi::StationaryObject_Type_TYPE_PYLON:
            objType = ObjectType::Pylon;
            break;
        case osi::StationaryObject_Type_TYPE_REFLECTOR_POST:
            objType = ObjectType::ReflectorPost;
            break;
        case osi::StationaryObject_Type_TYPE_DELINEATOR:
            objType = ObjectType::Delineator;
            break;
        default:
            break;
    }

    return objType;
}

ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi::TrafficSign_Type objectType)
{
    return ObjectType::TrafficSign;
}


ObjectType
OsiParser::GetObjectTypeFromOsiObjectType(const osi::TrafficLight_Type objectType)
{
    return ObjectType::TrafficLight;
}

ObjectType
OsiParser::GetObjectTypeFromHighestProbability(const osi::DetectedObject_ClassProbability& classProbability)
{
    ObjectType objectType = ObjectType::UnknownObject;
    float highestProbability = -1;

    if (classProbability.prob_bicycle() > highestProbability)
    {
        objectType = ObjectType::Bicycle;
        highestProbability = classProbability.prob_bicycle();
    }
    if (classProbability.prob_car() > highestProbability)
    {
        objectType = ObjectType::Car;
        highestProbability = classProbability.prob_car();
    }
    if (classProbability.prob_motorbike() > highestProbability)
    {
        objectType = ObjectType::MotorBike;
        highestProbability = classProbability.prob_motorbike();
    }
    if (classProbability.prob_pedestrian() > highestProbability)
    {
        objectType = ObjectType::Pedestrian;
        highestProbability = classProbability.prob_pedestrian();
    }
    if (classProbability.prob_stationary() > highestProbability)
    {
        objectType = ObjectType::Building;
        highestProbability = classProbability.prob_stationary();
    }
    if (classProbability.prob_truck() > highestProbability)
    {
        objectType = ObjectType::Truck;
        highestProbability = classProbability.prob_truck();
    }
    if (classProbability.prob_unknown() > highestProbability)
    {
        objectType = ObjectType::UnknownObject;
        highestProbability = classProbability.prob_unknown();
    }

    return objectType;
}

