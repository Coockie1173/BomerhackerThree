#include "layerfilehandler.h"
#include <QList>
#include <QDataStream>

LayerfileHandler::LayerfileHandler(QObject *parent)
    : QObject{parent}
{
    this->LFI = new QList<QList<LayerFileItem*>*>();
}

void LayerfileHandler::LoadLayerFile(QByteArray RawData)
{
    QDataStream ds(RawData);
    ds.setByteOrder(QDataStream::BigEndian);
    this->LoadedFile = new Layerfile();

    ds >> this->LoadedFile->Header;
    ds >> this->LoadedFile->DeathOffset;

    this->LoadedFile->Sections = new QList<Section*>();
    char AmmSections = RawData[0];

    for(int ParsedSections = 0; ParsedSections < AmmSections; ParsedSections++)
    {
        Section* Cursec = new Section();
        Cursec->SubSections = new QList<SubSection*>();
        ds >> Cursec->AmmPartsInSubsections;
        ds >> Cursec->AmmSubsections;
        ds >> Cursec->HeightOffset;
        Cursec->SubsectionLength = 0x80;

        for(int S = 0; S < (0xC00 / Cursec->SubsectionLength); S++)
        {
            SubSection* s = new SubSection();
            ds >> s->HeaderbyteOne;
            ds >> s->HeaderbyteTwo;
            s->Data = new QList<quint16>();
            for(int i = 0; i < Cursec->SubsectionLength; i += 2)
            {
                quint16 buf;
                ds >> buf;
                s->Data->append(buf);
            }
            Cursec->SubSections->append(s);
        }
        this->LoadedFile->Sections->append(Cursec);
    }
}
