///
/// @file
/// @copyright Copyright (C) 2017, Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
///
/// @brief
///
#ifndef OSI_VISUALIZER_CUSTOM_WIDGET_ITEM_H
#define OSI_VISUALIZER_CUSTOM_WIDGET_ITEM_H
#include "globject.h"
#include "types.h"
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
#endif  // OSI_VISUALIZER_CUSTOM_WIDGET_ITEM_H