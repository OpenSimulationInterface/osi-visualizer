///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#ifndef OSI_VISUALIZER_OSIPARSER_H
#define OSI_VISUALIZER_OSIPARSER_H
#include "global.h"
#include "mainwindow.h"
#include "osi_sensordata.pb.h"
#include "types.h"
#include <QList>
#include <QObject>
#include <QVector3D>

class OsiParser : public QObject
{
    Q_OBJECT

  public:
    OsiParser(const AppConfig& config);

    void CancelSaveOSIMessage();

  signals:
    void MessageParsed(const Message& message, const LaneMessage& laneMessage);
    void EnableExport(bool enable);
    void SaveOSIMsgOverflow(int osiMsgSaveThreshold);

  public slots:
    void ExportOsiMessage();
    void ParseReceivedSDMessage(const osi3::SensorData& sd);
    void ParseReceivedSVMessage(const osi3::SensorView& sv);

  private:
    template <typename T>
    void LocalMonitor(const T& data);

    void ParseGroundtruth(const osi3::GroundTruth& groundTruth, Message& objectMessage, LaneMessage& laneMessage);

    void ParseGroundtruthMovingObject(Message& objectMessage,
                                      const osi3::BaseMoving& baseObject,
                                      const ObjectType objectType,
                                      const QString& idStr);

    void ParseGroundtruthStationaryObject(Message& objectMessage,
                                          const osi3::BaseStationary& baseObject,
                                          const ObjectType objectType,
                                          const QString& idStr);

    void ParseSensorData(const osi3::SensorData& sensorData, Message& objectMessage, LaneMessage& laneMessage);

    void ParseSensorDataMovingObject(Message& objectMessage,
                                     const osi3::BaseMoving& baseObject,
                                     const ObjectType objectType,
                                     const QString& idStr);

    void ParseSensorDataStationaryObject(Message& objectMessage,
                                         const osi3::BaseStationary& baseObject,
                                         const ObjectType objectType,
                                         const QString& idStr);

    ObjectType GetObjectTypeFromOsiObjectType(const osi3::MovingObject_VehicleClassification_Type& vehicleType);
    ObjectType GetObjectTypeFromOsiObjectType(const osi3::MovingObject_Type& objectType);
    ObjectType GetObjectTypeFromOsiObjectType(const osi3::StationaryObject_Classification_Type& objectType);
    ObjectType GetObjectTypeFromOsiObjectType(const osi3::TrafficSign_MainSign_Classification_Type& objectType);
    ObjectType GetObjectTypeFromOsiObjectType(const osi3::TrafficLight_Classification_Mode& objectType);

    bool isFirstMessage_;

    bool startSaveOSIMsg_;
    std::string osiMsgString_;
    int osiMsgNumber_;

    const AppConfig& config_;
};

#endif  // OSI_VISUALIZER_OSIPARSER_H
