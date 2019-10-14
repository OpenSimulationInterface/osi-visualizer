#include "lane.h"

Lane::Lane(int id, const QVector<QVector<QVector3D> >& laneMarkings, QOpenGLFunctions_4_3_Core* functions)
    : laneId_(id), isVisible_(true), glLaneMarkings_(), functions_(functions)
{
    for (const auto& laneMarkers : laneMarkings)
    {
        AddLaneMarking(laneMarkers);
    }
}

void Lane::UpdateLaneMarkings(const QVector<QVector<QVector3D> >& laneMarkings)
{
    QVector<GLLaneMarking*> candidates(glLaneMarkings_);

    for (const auto& laneMarkers : laneMarkings)
    {
        GLLaneMarking* candidate = nullptr;

        if (candidates.size() > 0)
        {
            candidate = candidates.first();
            for (int i = 0; i < candidates.size(); ++i)
            {
                if (candidates.at(i)->vertices_.size() == laneMarkers.size())
                {
                    candidate = candidates.at(i);
                    break;
                }
            }
        }

        if (candidate)
        {
            // TODO: Free the memory currently occupied by candidate->vertices?
            candidate->vertices_ = laneMarkers;
            candidate->UpdateVertexBuffer();
            // qDebug() << "Updating buffer";

            candidate->isVisible_ = true;
            candidates.removeOne(candidate);
        }
        else
        {
            // qDebug() << "New buffer";
            AddLaneMarking(laneMarkers);
        }
    }

    // Don't remove the unused lane markings, just hide them

    foreach (GLLaneMarking* candidate, candidates)
    {
        candidate->isVisible_ = false;
    }
}

void Lane::AddLaneMarking(const QVector<QVector3D>& laneMarkers)
{
    GLLaneMarking* marking = new GLLaneMarking(functions_, laneMarkers);
    marking->Init();
    marking->SetColor(Qt::white);
    glLaneMarkings_.append(marking);
}
