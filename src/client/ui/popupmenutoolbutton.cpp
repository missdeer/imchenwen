#include <QAction>
#include "popupmenutoolbutton.h"

PopupMenuToolButton::PopupMenuToolButton(QWidget *parent)
    : QToolButton (parent)
{
    setPopupMode(QToolButton::MenuButtonPopup);
    //connect(this, &QToolButton::triggered, this, &QToolButton::setDefaultAction);
}
