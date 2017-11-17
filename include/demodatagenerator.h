///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "osiobjectstub.h"
#include<QList>

class DemoDataGenerator
{
    public:
        DemoDataGenerator(QList<OSIObjectStub*>* vehicles);
        void Update();

    private:
        QList<OSIObjectStub*>* vehicles;
};
