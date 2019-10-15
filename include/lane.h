///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_LANE_H
#define OSI_VISUALIZER_LANE_H
#include "gllanemarking.h"
#include <QOpenGLFunctions_4_3_Core>
#include <QVector3D>

class Lane
{
  public:
    Lane(int id, const QVector<QVector<QVector3D> >& laneMarkings, QOpenGLFunctions_4_3_Core* functions);

    void UpdateLaneMarkings(const QVector<QVector<QVector3D> >& laneMarkings);

    int laneId_;
    bool isVisible_;
    QVector<GLLaneMarking*> glLaneMarkings_;

  private:
    void AddLaneMarking(const QVector<QVector3D>& laneMarkers);

    QOpenGLFunctions_4_3_Core* functions_;
};
#endif  // OSI_VISUALIZER_LANE_H