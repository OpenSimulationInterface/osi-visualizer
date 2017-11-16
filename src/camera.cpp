#define _USE_MATH_DEFINES

#include <math.h>
#include "camera.h"

Camera::Camera(CameraPerspective perspective)
    : viewMatrix_()
    , trackedObject_(nullptr)
    , resetPosition_()

    , minY_(1)
    , orientation_(0)
    , position_()
    , target_()
    , up_()
    , right_()
    , initialUp_()
    , resetOrientation_(0)
{
    if (perspective == Default)
    {
        up_.setY(1);
        right_.setX(1);
        target_.setZ(-1);
    }
    else if (perspective == BirdsEye)
    {
        up_.setX(1);
        right_.setZ(1);
        target_.setY(-1);
    }

    initialUp_ = QVector4D(up_, 1);

    UpdateViewMatrix();
}

void
Camera::Translate(float x, float y, float z)
{
    Translate(QVector3D(x, y, z));
}

void
Camera::Translate(QVector3D translation)
{
    SetPosition(position_ + translation);
}

void
Camera::SetPosition(float x , float y, float z)
{
    SetPosition(QVector3D(x, y, z));
}

void
Camera::SetPosition(QVector3D position)
{
    if (position.y() >= minY_)
    {
        position_ = position;
        UpdateViewMatrix();
    }
}

void
Camera::SetToObjectPosition(GLObject* object)
{
    SetPosition(object->GetPosition());
}

void
Camera::UpdateRight()
{
    right_ = -QVector3D::crossProduct(up_, target_).normalized();
}

void
Camera::UpdateUp()
{
    double cosp = cos(orientation_);
    double sinp = sin(orientation_);
    // TODO: Find init ctor for QMatrix3x3 (maybe use << ?)
    QMatrix4x4 rotY(cosp, 0, sinp, 0,
                    0, 1, 0, 0,
                    -sinp, 0, cosp, 0,
                    0, 0, 0, 1);

    // Don't accumulate the rotations on "up_", small errors will increase over time
    // Always use the initial up_ vector and rotate it to the current absolute orientation
    up_ = (rotY * initialUp_).toVector3D();
}

void
Camera::SetOrientation(float orientation)
{
    orientation = fmod(orientation, 2 * M_PI);
    orientation_ = orientation;

    UpdateUp();
    UpdateRight();
    UpdateViewMatrix();
}

void
Camera::RotateAroundYAxis(float rotationRad)
{
    SetOrientation(orientation_ + rotationRad);
}

void
Camera::ResetPosition()
{
    SetPosition(resetPosition_);
}

void
Camera::ResetOrientation()
{
    SetOrientation(resetOrientation_);
}

void
Camera::ResetAll()
{
    trackedObject_ = nullptr;
    ResetPosition();
    ResetOrientation();
}

void
Camera::SetUp(QVector3D up)
{
    up_ = up;
    UpdateViewMatrix();
}

void
Camera::UpdateViewMatrix()
{
    viewMatrix_.setToIdentity();
    viewMatrix_.lookAt(position_, position_ + target_, up_);
}

/*
void Camera::SetTarget(float x, float y, float z)
{
    SetTarget(QVector3D(x, y, z));
}

void Camera::SetTarget(QVector3D target)
{
    this->target = target;
    UpdateViewMatrix();
}
*/
