///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_APP_CONFIG_H
#define OSI_VISUALIZER_APP_CONFIG_H
#include "types.h"
#include <QMap>
#include <QString>

class AppConfig
{
  public:
    AppConfig(QString fileName);
    bool Load();
    bool Save();

    QString ch1IPAddress_;
    QString ch1PortNum_;
    DataType ch1DataType_;
    bool ch1FMURxCheck_;
    QString ch1LoadFMURx_;
    QString ch1LoadFile_;
    DataType ch1PlaybackDataType_;
    int ch1DeltaDelay_;
    bool ch1EnableSendOut_;
    QString ch1SendOutPortNum_;
    bool ch1FMUTxCheck_;
    QString ch1LoadFMUTx_;
    bool ch1ShowFOV_;
    float ch1MinRadius_;
    float ch1MaxRadius_;
    float ch1AzimuthPos_;
    float ch1AzimuthNeg_;

    QString ch2IPAddress_;
    QString ch2PortNum_;
    DataType ch2DataType_;
    bool ch2FMURxCheck_;
    QString ch2LoadFMURx_;
    QString ch2LoadFile_;
    DataType ch2PlaybackDataType_;
    int ch2DeltaDelay_;
    bool ch2EnableSendOut_;
    QString ch2SendOutPortNum_;
    bool ch2FMUTxCheck_;
    QString ch2LoadFMUTx_;
    bool ch2ShowFOV_;
    float ch2MinRadius_;
    float ch2MaxRadius_;
    float ch2AzimuthPos_;
    float ch2AzimuthNeg_;

    bool combineChannel_;
    bool showGrid_;
    bool showObjectDetails_;
    bool lockCamera_;
    LaneType laneType_;
    QMap<ObjectType, QColor> typeColors_;

    int osiMsgSaveThreshold_;
    QString srcPath_;

  private:
    QString configFileName_;
    QString config_file_path_{"."};
};

#endif  // OSI_VISUALIZER_APP_CONFIG_H
