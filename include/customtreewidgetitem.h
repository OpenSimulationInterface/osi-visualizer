///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///

#pragma once
#include "types.h"
#include "globject.h"
#include <QElapsedTimer>
#include <QTreeWidgetItem>

class CustomTreeWidgetItem : public QTreeWidgetItem
{
    public:
        CustomTreeWidgetItem(GLObject* glObject);

        bool ignoreClick_;
        GLObject* glObject_;
        ObjectType objectType_;
        QElapsedTimer* lastClickTimer_;
};
