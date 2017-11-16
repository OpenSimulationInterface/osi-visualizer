///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "globject.h"

class GLVehicle : public GLObject
{
    public:
        GLVehicle(QOpenGLFunctions_4_3_Core* functions, QString id, float xExtend, float zExtend);
};
