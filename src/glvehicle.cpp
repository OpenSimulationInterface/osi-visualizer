#include "glvehicle.h"
#include <QVector>
#include <QOpenGLFunctions>

GLVehicle::GLVehicle(QOpenGLFunctions_4_3_Core* functions,
                     QString id,
                     float xExtend,
                     float zExtend)
    : GLObject(GL_QUADS, functions, id)
{
    float xHalfExtend = xExtend / 2.0f;
    float zHalfExtend = zExtend / 2.0f;

    vertices_ << QVector3D(-xHalfExtend, 0, zHalfExtend)
              << QVector3D(xHalfExtend, 0, zHalfExtend)
              << QVector3D(xHalfExtend, 0, -zHalfExtend)
              << QVector3D(-xHalfExtend, 0, -zHalfExtend);
}

GLVehicle::GLVehicle(QOpenGLFunctions_4_3_Core* functions,
                     QString id,
                     QVector3D v0,
                     QVector3D v1,
                     QVector3D v2,
                     QVector3D v3)
    : GLObject(GL_QUADS, functions, id)
{
    vertices_ << v0 << v1 << v2 << v3;
}



