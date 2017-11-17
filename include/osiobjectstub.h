///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include <QVector3D>

enum ObjectType
{
    Vehicle,
    TrafficSign
};

class OSIObjectStub
{
    public:
        OSIObjectStub();
        OSIObjectStub(int id, ObjectType type, QVector3D position, float orientation);

        int id;
        ObjectType type;
        QVector3D position;
        float orientation;
};

