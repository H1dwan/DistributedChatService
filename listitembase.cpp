#include "listitembase.h"
#include <QStyleOption>
#include <QPainter>

ListItemBase::ListItemBase(QWidget* parent) : QWidget(parent)
{}

void ListItemBase::SetItemType(ListItemType item_type)
{
    _item_type = item_type;
}

ListItemType ListItemBase::GetItemType() const
{
    return _item_type;
}

void ListItemBase::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    QWidget::paintEvent(event);
}
