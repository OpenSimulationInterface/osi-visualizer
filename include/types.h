///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once

#include "osi_common.pb.h"
#include <QVector>
#include <QVector3D>


#include <memory>


enum class DataType: int
{
    Groundtruth = 0,
    SensorData
};

enum class LaneType: int
{
    BoundaryLanes = 0,
    CenterLanes
};

enum class ObjectType: int
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
    bool isEgoVehicle;
    float orientation;
    QVector3D position;
    QVector3D realPosition;
    QVector3D velocitie;
    QVector3D acceleration;
    osi::Dimension3d dimension;
};
using Message = QVector<MessageStruct>;


struct LaneStruct
{
    int id;
    QVector<QVector<QVector3D> > centerLanes;
    QVector<QVector<QVector3D> > boundaryLanes;
};
using LaneMessage = QVector<LaneStruct>;


