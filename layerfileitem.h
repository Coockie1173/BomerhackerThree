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
    int X;
    int Y;

    void RefreshVisual();

signals:
    // Declare the signal
    void clicked(LayerFileItem *item, int DataItem);
    void RightClicked(LayerFileItem *item);

private:

};

#endif // LAYERFILEITEM_H
