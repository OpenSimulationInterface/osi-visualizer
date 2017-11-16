#include "gltrafficsign.h"
#include <QtMath>

GLTrafficSign::GLTrafficSign(QOpenGLFunctions_4_3_Core* functions,
                             QString id)
    : GLObject(GL_TRIANGLE_FAN, functions, id)
{
    int slices = 30;
    float radius = 0.3f;
    float angle = 0;
    float angleStep = 2 * M_PI / slices;

    for (int i = 0; i <= slices; ++i)
    {
        float x = radius * sin(angle);
        float z = radius * cos(angle);
        vertices_ << QVector3D(x, 0, z);

        angle += angleStep;
    }
}
