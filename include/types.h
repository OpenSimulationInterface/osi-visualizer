///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_TYPES_H
#define OSI_VISUALIZER_TYPES_H

#include "osi_common.pb.h"
#include <QVector3D>
#include <QVector>

#include <memory>

enum class DataType : int
{
    SensorView = 0,
    SensorData
};

enum class LaneType : int
{
    BoundaryLanes = 0,
    CenterLanes
};

enum class ObjectType : int
{
    UnknownObject = 0,
    OtherObject,
    Car,
    Truck,
    MotorBike,
    Bicycle,
    Trailer,
    Pedestrian,
    Animal,
    Bridge,
    Building,
    Pylon,
    ReflectorPost,
    Delineator,
    TrafficSign,
    TrafficLight,
    None
};

struct MessageStruct
{
    QString id;
    QString name;
    ObjectType type;
    float orientation;
    QVector3D position;
    QVector3D realPosition;
    QVector3D velocitie;
    QVector3D acceleration;
    osi3::Dimension3d dimension;
    QVector<QVector3D> basePoly;
};
using Message = QVector<MessageStruct>;

struct LaneStruct
{
    int id;
    QVector<QVector<QVector3D> > centerLanes;
    QVector<QVector<QVector3D> > boundaryLanes;
};
using LaneMessage = QVector<LaneStruct>;
#endif  // OSI_VISUALIZER_TYPES_H