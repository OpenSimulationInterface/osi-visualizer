///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "globject.h"


class GLLaneMarking : public GLObject
{
    public:
         GLLaneMarking(QOpenGLFunctions_4_3_Core* functions,
                       const QVector<QVector3D>& laneMarkers);
};
