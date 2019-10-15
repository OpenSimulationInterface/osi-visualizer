#include "glgrid.h"

GLGrid::GLGrid(QOpenGLFunctions_4_3_Core* functions, const QString& srcPath) : GLObject(GL_QUADS, functions)
{
    float gridSize = 40000;
    float texCoordExtends = 16000;

    vertices_ << QVector3D(-gridSize, 0, -gridSize) << QVector3D(gridSize, 0, -gridSize)
              << QVector3D(gridSize, 0, gridSize) << QVector3D(-gridSize, 0, gridSize);

    texCoords_ << QVector2D(-texCoordExtends, -texCoordExtends) << QVector2D(texCoordExtends, -texCoordExtends)
               << QVector2D(texCoordExtends, texCoordExtends) << QVector2D(-texCoordExtends, texCoordExtends);

    QImage gridPattern("./resources/Images/Grid.png");
    SetTexture(gridPattern, true);
}
