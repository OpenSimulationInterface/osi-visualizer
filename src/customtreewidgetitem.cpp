#include "customtreewidgetitem.h"

CustomTreeWidgetItem::CustomTreeWidgetItem(GLObject* glObject)
    : ignoreClick_(false), glObject_(glObject), objectType_(), lastClickTimer_(nullptr)
{
}
