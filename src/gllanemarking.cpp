#include "gllanemarking.h"

GLLaneMarking::GLLaneMarking(QOpenGLFunctions_4_3_Core* functions,
                             const QVector<QVector3D>& laneMarkers)
    : GLObject(GL_LINE_STRIP, functions, "", GL_STREAM_DRAW)
{
    vertices_ = laneMarkers;
}
