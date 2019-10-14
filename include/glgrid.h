///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_GL_GRID_H
#define OSI_VISUALIZER_GL_GRID_H
#include "globject.h"

class GLGrid : public GLObject
{
  public:
    GLGrid(QOpenGLFunctions_4_3_Core* functions, const QString& srcPath);
};
#endif  // OSI_VISUALIZER_GL_GRID_H