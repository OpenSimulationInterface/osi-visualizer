///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "globject.h"

class GLTrafficSign : public GLObject
{
    public:
        GLTrafficSign(QOpenGLFunctions_4_3_Core* functions, QString id);
};
