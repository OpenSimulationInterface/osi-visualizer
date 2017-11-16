///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "globject.h"

class GLGrid : public GLObject
{
    public:
        GLGrid(QOpenGLFunctions_4_3_Core* functions, const QString& srcPath);
};
