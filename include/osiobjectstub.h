///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_OSI_OBJECT_STUB_H
#define OSI_VISUALIZER_OSI_OBJECT_STUB_H
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
#endif  // OSI_VISUALIZER_OSI_OBJECT_STUB_H