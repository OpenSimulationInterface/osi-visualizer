///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#ifndef OSIPARSER_H
#define OSIPARSER_H
#include "osi_sensordata.pb.h"
#include "global.h"
#include "types.h"
#include "mainwindow.h"
#include <QObject>
#include <QVector3D>
#include <QList>


class OsiParser : public QObject
{
    Q_OBJECT

    public:
        OsiParser(const int osiMsgSaveThreshold);

        void CancelSaveOSIMessage();

        signals:
            void MessageParsed(const Message& message,
                               const LaneMessage& laneMessage);
            void EnableExport(bool enable);
            void SaveOSIMsgOverflow(int osiMsgSaveThreshold);

    public slots:
        void ExportOsiMessage();
        void ParseReceivedMessage(const osi::SensorData& SensorData,
                                  const DataType datatype);

    private:

        void ParseGroundtruth(Message& objectMessage, LaneMessage& laneMessage);

        void ParseGroundtruthMovingObject(Message& objectMessage,
                                          const osi::BaseMoving& baseObject,
                                          const ObjectType objectType,
                                          const QString& idStr,
                                          const bool isEgoVehicle);

        void ParseGroundtruthStationaryObject(Message& objectMessage,
                                              const osi::BaseStationary& baseObject,
                                              const ObjectType objectType,
                                              const QString& idStr,
                                              const bool isEgoVehicle);

        void ParseSensorData(Message& objectMessage,
                             LaneMessage& laneMessage);

        void ParseSensorDataMovingObject(Message& objectMessage,
                                         const osi::BaseMoving& baseObject,
                                         const ObjectType objectType,
                                         const QString& idStr);

        void ParseSensorDataStationaryObject(Message& objectMessage,
                                             const osi::BaseStationary& baseObject,
                                             const ObjectType objectType,
                                             const QString& idStr);

        ObjectType GetObjectTypeFromOsiVehicleType(const osi::Vehicle_Type vehicleType);
        ObjectType GetObjectTypeFromOsiObjectType(const osi::MovingObject_Type objectType);
        ObjectType GetObjectTypeFromOsiObjectType(const osi::StationaryObject_Type objectType);
        ObjectType GetObjectTypeFromOsiObjectType(const osi::TrafficSign_Type objectType);
        ObjectType GetObjectTypeFromOsiObjectType(const osi::TrafficLight_Type objectType);
        ObjectType GetObjectTypeFromHighestProbability(const osi::DetectedObject_ClassProbability& classProbability);



        osi::GroundTruth currentGroundTruth_;
        osi::SensorData currentSensorData_;
        DataType currentDataType_;

        bool isFirstMessage_;

        bool startSaveOSIMsg_;
        std::string osiMsgString_;
        int osiMsgNumber_;
        const int osiMsgSaveThreshold_;

};

#endif // OSIPARSER_H
