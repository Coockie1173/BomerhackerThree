#include "layerfileitem.h"
#include <QBrush>
#include <QDebug>
#include <QColor>
#include <QRgb>
#include <QGraphicsSceneMouseEvent>

LayerFileItem::LayerFileItem()
{

}

void LayerFileItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Handle left mouse button click
        emit clicked(this, DataItem);
    }
    else if (event->button() == Qt::RightButton)
    {
        // Handle right mouse button click
        emit RightClicked(this);
    }
}


void LayerFileItem::RefreshVisual()
{
    switch(this->DataItem & 0b00001111)
    {
    case 0:
    {
        //air
        setBrush(Qt::blue);
        break;
    }
    case 1:
    {
        //floor
        QRgb col = 0xFFA500;
        QColor oreng(col);

        setBrush(oreng);
        break;
    }
    case 2:
    {
        //TransitionSquare
        QColor oreng(Qt::green);
        setBrush(oreng);
        break;
    }
    case 3:
    {
        //ramp down
        setBrush(Qt::red);
        break;
    }
    case 4:
    {
        //ramp left
        setBrush(Qt::red);
        break;
    }
    case 5:
    {
        //ramp right
        setBrush(Qt::red);
        break;
    }
    case 6:
    {
        //ramp up
        setBrush(Qt::red);
        break;
    }
    case 9:
    {
        setBrush(Qt::darkRed);
        break;
    }
    case 0xF:
    {
        setBrush(Qt::gray);
        break;
    }
    }
}
