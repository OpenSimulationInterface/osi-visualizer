///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "types.h"
#include <QString>
#include <QMap>

class AppConfig
{
    public:

        AppConfig(QString fileName);
        bool Load();
        bool Save();

        QString  ch1IPAddress_;
        QString  ch1PortNum_;
        DataType ch1DataType_;
        QString  ch1LoadFile_;
        DataType ch1PlaybackDataType_;
        int      ch1DeltaDelay_;

        QString  ch2IPAddress_;
        QString  ch2PortNum_;
        DataType ch2DataType_;
        QString  ch2LoadFile_;
        DataType ch2PlaybackDataType_;
        int      ch2DeltaDelay_;

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
};

