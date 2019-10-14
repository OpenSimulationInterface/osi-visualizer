///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_GL_VEHICLE_H
#define OSI_VISUALIZER_GL_VEHICLE_H
#include "globject.h"

class GLVehicle : public GLObject
{
  public:
    GLVehicle(QOpenGLFunctions_4_3_Core* functions, QString id, float xExtend, float zExtend);

    GLVehicle(QOpenGLFunctions_4_3_Core* functions, QString id, QVector3D v0, QVector3D v1, QVector3D v2, QVector3D v3);
};
#endif  // OSI_VISUALIZER_GL_VEHICLE_H