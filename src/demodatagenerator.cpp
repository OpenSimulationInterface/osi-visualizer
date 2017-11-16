#include "demodatagenerator.h"

DemoDataGenerator::DemoDataGenerator(QList<OSIObjectStub*>* vehicles)
{
    this->vehicles = vehicles;
}

void DemoDataGenerator::Update()
{
    foreach (OSIObjectStub* vehicle, *vehicles) {
        vehicle->orientation += 0.01f * vehicle->id;
        vehicle->position.setX(vehicle->position.x() + 0.002f * vehicle->id * (vehicle->id % 2 ? 1 : -1));
        vehicle->position.setZ(vehicle->position.z() + 0.002f * vehicle->id * (vehicle->id % 2 ? 1 : -1));
    }
}
