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
