#ifndef LAYERFILEITEM_H
#define LAYERFILEITEM_H

#include <QGraphicsRectItem>
#include <QObject>
#include <QString>

class LayerFileItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    LayerFileItem();

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    int DataItem;
    int DataPos;
    int Subsection;
    int Layer;

    void RefreshVisual();

private:

};

#endif // LAYERFILEITEM_H
