///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once

#include "globject.h"

#include <QDialog>
#include <QLabel>


class DisplayObjectDialog: public QDialog
{
    Q_OBJECT

public:
    explicit DisplayObjectDialog(QWidget* parent);

    void UpdateObjectInformation(const GLObject& object);

private:
    QLabel* positionX_;
    QLabel* positionY_;
    QLabel* positionZ_;
    QLabel* velocityX_;
    QLabel* velocityY_;
    QLabel* velocityZ_;
    QLabel* accelerationX_;
    QLabel* accelerationY_;
    QLabel* accelerationZ_;
};

