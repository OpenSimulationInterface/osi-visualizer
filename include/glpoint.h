///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "globject.h"

class GLPoint : public GLObject
{
    public:
        GLPoint(QOpenGLFunctions_4_3_Core* functions, QString id);
};
