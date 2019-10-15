#include "glfieldofview.h"
#include <QtMath>

GLFieldOfView::GLFieldOfView(QOpenGLFunctions_4_3_Core* functions,
                             const float minRadius,
                             const float maxRadius,
                             const float azimuthPosAngle,
                             const float azimuthNegAngle)
    : GLObject(GL_LINE_LOOP, functions)
{
    UpdateParameter(minRadius, maxRadius, azimuthPosAngle, azimuthNegAngle);
}

void GLFieldOfView::UpdateParameter(const float minRadius,
                                    const float maxRadius,
                                    const float azimuthPosAngle,
                                    const float azimuthNegAngle)
{
    vertices_.clear();

    float angle = azimuthNegAngle;

    int minSlices = 50;
    int maxSlices = 200;
    float minAngleSteps = (azimuthPosAngle - azimuthNegAngle) / (float)minSlices;
    float maxAngleSteps = (azimuthPosAngle - azimuthNegAngle) / (float)maxSlices;

    for (int i = 0; i <= minSlices; ++i)
    {
        float x = minRadius * sin(angle);
        float z = minRadius * cos(angle);
        vertices_ << QVector3D(x, 0, z);

        angle += minAngleSteps;
    }

    QVector<QVector3D> outCurve;
    angle = azimuthNegAngle;

    for (int i = 0; i <= maxSlices; ++i)
    {
        float x = maxRadius * sin(angle);
        float z = maxRadius * cos(angle);
        outCurve << QVector3D(x, 0, z);

        angle += maxAngleSteps;
    }

    for (int i = outCurve.size() - 1; i >= 0; --i)
    {
        vertices_ << outCurve[i];
    }
}
