#include "gltriangle.h"

GLTriangle::GLTriangle(QOpenGLFunctions_4_3_Core* functions) : GLObject(GL_TRIANGLES, functions, "")
{
    vertices_ << QVector3D(-1.0f, 0, -0.9f);
    vertices_ << QVector3D(-1.0f, 0, 0.9f);
    vertices_ << QVector3D(1.0f, 0, 0);
}
