///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_GL_TRIANGLE_H
#define OSI_VISUALIZER_GL_TRIANGLE_H
#include "globject.h"

class GLTriangle : public GLObject
{
  public:
    GLTriangle(QOpenGLFunctions_4_3_Core* functions);
};
#endif  // OSI_VISUALIZER_GL_TRIANGLE_H