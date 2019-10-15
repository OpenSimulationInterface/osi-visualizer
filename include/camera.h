///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_CAMERA_H
#define OSI_VISUALIZER_CAMERA_H
#include "globject.h"
#include <QMatrix4x4>
#include <QVector3D>

typedef enum
{
    Default,
    BirdsEye
} CameraPerspective;

class Camera
{
  public:
    explicit Camera(CameraPerspective perspective = Default);

    void UpdateUp();
    void SetUp(QVector3D up);
    QVector3D GetUp() { return up_; }

    void UpdateRight();
    QVector3D GetRight() { return right_; }

    void SetToObjectPosition(GLObject* object);
    void SetPosition(float x, float y, float z);
    void SetPosition(QVector3D position);
    QVector3D GetPosition() { return position_; }

    void SetOrientation(float orientation);
    float GetOrientation() { return orientation_; }

    void ResetAll();
    void ResetPosition();
    void ResetOrientation();

    void Translate(QVector3D translation);
    void Translate(float x, float y, float z);

    void RotateAroundYAxis(float rotationRad);

    // void SetTarget(QVector3D target);
    // void SetTarget(float x, float y, float z);

    QMatrix4x4 viewMatrix_;
    GLObject* trackedObject_;
    QVector3D resetPosition_;

  private:
    void UpdateViewMatrix();

    float minY_;
    float orientation_;
    QVector3D position_;
    QVector3D target_;
    QVector3D up_;
    QVector3D right_;
    QVector4D initialUp_;
    float resetOrientation_;
};
#endif  // OSI_VISUALIZER_CAMERA_H