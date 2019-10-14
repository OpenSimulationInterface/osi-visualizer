///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_GL_LANE_MARKING_H
#define OSI_VISUALIZER_GL_LANE_MARKING_H
#include "globject.h"

class GLLaneMarking : public GLObject
{
  public:
    GLLaneMarking(QOpenGLFunctions_4_3_Core* functions, const QVector<QVector3D>& laneMarkers);
};
#endif  // OSI_VISUALIZER_GL_LANE_MARKING_H