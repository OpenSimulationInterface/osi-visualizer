///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_GL_POINT_H
#define OSI_VISUALIZER_GL_POINT_H
#include "globject.h"

class GLPoint : public GLObject
{
  public:
    GLPoint(QOpenGLFunctions_4_3_Core* functions, QString id);
};
#endif  // OSI_VISUALIZER_GL_POINT_H