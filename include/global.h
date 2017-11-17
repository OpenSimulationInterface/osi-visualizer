///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "types.h"
#include <QString>
#include <QList>

class Global
{
    public:
        static QString GetObjectTypeName(const ObjectType& type, bool plural = false)
        {
            QString name;
            switch (type) {
                case ObjectType::UnknownObject:
                    name = "UnknownObject";
                    break;
                case ObjectType::OtherObject:
                    name = "OtherObject";
                    break;
                case ObjectType::Car:
                    name = "Car";
                    break;
                case ObjectType::Truck:
                    name = "Truck";
                    break;
                case ObjectType::MotorBike:
                    name = "MotorBike";
                    break;
                case ObjectType::Bicycle:
                    name = "Bicycle";
                    break;
                case ObjectType::Trailer:
                    name = "Trailer";
                    break;
                case ObjectType::Pedestrian:
                    name = "Pedestrian";
                    break;
                case ObjectType::Animal:
                    name = "Animal";
                    break;
                case ObjectType::Bridge:
                    name = "Bridge";
                    break;
                case ObjectType::Building:
                    name = "Building";
                    break;
                case ObjectType::Pylon:
                    name = "Pylon";
                    break;
                case ObjectType::ReflectorPost:
                    name = "ReflectorPost";
                    break;
                case ObjectType::Delineator:
                    name = "Delineator";
                    break;
                case ObjectType::TrafficSign:
                    name = "TrafficSign";
                    break;
                case ObjectType::TrafficLight:
                    name = "TrafficLight";
                    break;
                default:
                    break;
            }

            if (plural && type != ObjectType::UnknownObject)
            {
                name += "s";
            }

            return name;

        }

        static QList<ObjectType> GetAllObjectTypes()
        {
            QList<ObjectType> types;
            types.append(ObjectType::UnknownObject);
            types.append(ObjectType::OtherObject);
            types.append(ObjectType::Car);
            types.append(ObjectType::Truck);
            types.append(ObjectType::MotorBike);
            types.append(ObjectType::Bicycle);
            types.append(ObjectType::Trailer);
            types.append(ObjectType::Pedestrian);
            types.append(ObjectType::Animal);
            types.append(ObjectType::Bridge);
            types.append(ObjectType::Building);
            types.append(ObjectType::Pylon);
            types.append(ObjectType::ReflectorPost);
            types.append(ObjectType::Delineator);
            types.append(ObjectType::TrafficSign);
            types.append(ObjectType::TrafficLight);

            return types;
        }
};
