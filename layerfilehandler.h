#ifndef LAYERFILEHANDLER_H
#define LAYERFILEHANDLER_H

#include <QObject>
#include <QList>
#include <QtEndian>
#include "layerfileitem.h"

class LayerfileHandler : public QObject
{
    Q_OBJECT
public:
    explicit LayerfileHandler(QObject *parent = nullptr);

    void LoadLayerFile(QByteArray RawData);

    struct SubSection
    {
    public:
        quint8 HeaderbyteOne;
        quint8 HeaderbyteTwo;
        QList<quint16>* Data;
    };
    struct Section
    {
    public:
        QList<SubSection*>* SubSections;
        quint8 AmmSubsections;
        quint8 AmmPartsInSubsections;
        quint8 HeightOffset;
        int SubsectionLength;
        int SubsectionOffset;
    };
    struct Layerfile
    {
    public:
        int Header;
        quint8 DeathOffset;
        QList<Section*>* Sections;        
    };

    Layerfile* LoadedFile;
    QList<QList<LayerFileItem*>*>* LFI;

signals:

};

#endif // LAYERFILEHANDLER_H
