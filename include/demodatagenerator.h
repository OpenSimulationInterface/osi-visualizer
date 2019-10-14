///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_DEMO_DATA_GENERATOR_H
#define OSI_VISUALIZER_DEMO_DATA_GENERATOR_H
#include "osiobjectstub.h"
#include <QList>

class DemoDataGenerator
{
  public:
    DemoDataGenerator(QList<OSIObjectStub*>* vehicles);
    void Update();

  private:
    QList<OSIObjectStub*>* vehicles;
};
#endif  // OSI_VISUALIZER_DEMO_DATA_GENERATOR_H
