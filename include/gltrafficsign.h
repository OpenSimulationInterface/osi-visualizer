///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_GL_TRAFFIC_SIGN_H
#define OSI_VISUALIZER_GL_TRAFFIC_SIGN_H
#include "globject.h"

class GLTrafficSign : public GLObject
{
  public:
    GLTrafficSign(QOpenGLFunctions_4_3_Core* functions, QString id);
};
#endif  // OSI_VISUALIZER_GL_TRAFFIC_SIGN_H