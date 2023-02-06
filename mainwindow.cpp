#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QFileInfo>
#include <algorithm>    // std::sort
#include "layerfileitem.h"
#include <QBuffer>
#include <QVector3D>
#include <QDataStream>
#define EXPORTSQUARESIZE 10

QGraphicsScene* LayerScene;
int SquareSize = 10;
int LayerLevel = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ToolsFolder = qApp->applicationDirPath() + "/BMTools/";
    Preferences = new PreferencesManager(qApp->applicationDirPath() + "/preferences.cfg");
    LFH = new LayerfileHandler();    

    ui->CurrentRomPathBox->setPlainText(Preferences->ROMPath);

    LayerScene = new QGraphicsScene();
    ui->graphicsView->setScene(LayerScene);

    QString fileName(":/FileList.txt");

    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly | QFile::Text)) {
        qDebug()<<"file not opened"<<endl;
    }
    else
    {
        while(!file.atEnd())
        {
            QString line = file.readLine();
            qDebug() << line;
            ui->DataFileListCombobox->addItem(line);
            if(line.contains("collision"))
            {
                ui->ColissionCombo->addItem(line);
            }
        }
    }

    for(int i = 0; i < 0x10; i++)
    {
        ui->GroundTypeComboBox->addItem(FloorIdentifier[i]);
        if(i == 9)
        {
            ui->ItemTypeComboBox->addItem(ItemIdentifier[i] + " - Playerspawn");
        }
        else
        {
            ui->ItemTypeComboBox->addItem(ItemIdentifier[i]);
        }
    }

    ui->EnemyCombobox->addItem("No enemy");

    for(int i = 1; i< 0x07; i++)
    {
        ui->EnemyCombobox->addItem("Enemy ID " + QString::number(i));
    }

    file.close();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_SetROMButton_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("ROMs (*.z64 *.rom)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        Preferences->ROMPath = fileNames[0];
        Preferences->WritePreferences();
        ui->CurrentRomPathBox->setPlainText(Preferences->ROMPath);
    }
}


void MainWindow::on_pushButton_clicked()
{
    ui->InjectProgressBar->setMaximum(50);
    ui->InjectProgressBar->setValue(0);

    QFileInfo fi(Preferences->ROMPath);
    QString tmpFolder = fi.absolutePath() + "/.tmp/";

    QDir dir(tmpFolder);
    dir.removeRecursively();

    QString ToolPath = ToolsFolder + "bm64romtool.exe";

    QStringList args;
    args << Preferences->ROMPath;
    args << tmpFolder;

    QProcess process;
    process.execute(ToolPath, args); //run decompressor
    process.exitCode();
    process.waitForFinished();

    ui->InjectProgressBar->setValue(10); //decompressed

    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("binaries (*.bin)"));
    dialog.setViewMode(QFileDialog::Detail);
    QStringList fileNames;
    if (dialog.exec())
    {
        fileNames = dialog.selectedFiles();
        QString NewFile = fileNames[0];
        QString OldFile = tmpFolder + "data/" + "file" + QString::number(ui->DataFileListCombobox->currentIndex()) + ".bin";

        QFile nf(OldFile);
        nf.remove();

        QFile::copy(NewFile, OldFile);
        ui->InjectProgressBar->setValue(20); //file overwritten

        QString NewROM = QFileDialog::getSaveFileName(this, "Save ROM", "ROM.z64", "ROM files (*.z64)");

        if(NewROM != "")
        {
            //path was set
            args.clear();
            args << tmpFolder;
            args << NewROM;
            process.execute(ToolPath, args);
            process.exitCode();
            process.waitForFinished();

            ui->InjectProgressBar->setValue(30); //rom written

            dir.removeRecursively(); //remove directory again
            ui->InjectProgressBar->setValue(40);

            QFile NR(NewROM);
            if(NR.open(QIODevice::ReadOnly))
            {
                QByteArray buf = NR.readAll();
                int NewSize = buf.size();
                while(NewSize % (0x1000 * 0x400) != 0)
                {
                    NewSize++;
                }
                buf.resize(NewSize);
                NR.close();
                if(NR.open(QIODevice::WriteOnly))
                {
                    NR.write(buf);
                    NR.close();
                }
            }

            ui->InjectProgressBar->setValue(50);
        }
    }
    else
    {
        ui->InjectProgressBar->setValue(0);
    }
}



void MainWindow::on_pushButton_2_clicked()
{
    QString SelectedObject = ui->ColissionCombo->currentText();
    int FileID = SelectedObject.mid(0,3).toInt();

    QFileInfo fi(Preferences->ROMPath);
    QString tmpFolder = fi.absolutePath() + "/.tmp/";

    QDir dir(tmpFolder);
    dir.removeRecursively();

    QString ToolPath = ToolsFolder + "bm64romtool.exe";

    QStringList args;
    args << Preferences->ROMPath;
    args << tmpFolder;

    QProcess process;
    process.execute(ToolPath, args); //run decompressor
    process.exitCode();
    process.waitForFinished();

    QString Wantedfile = tmpFolder + "data/" + "file" + QString::number(FileID) + ".bin";

    QFile file(Wantedfile);

    if(file.open(QIODevice::ReadOnly))
    {
        this->LFH->LoadLayerFile(file.readAll());
        this->ui->LayerSelectSlider->setValue(0);
        this->ui->LayerSelectSlider->setMaximum(this->LFH->LoadedFile->Sections->count() - 1);
    }

    dir.removeRecursively();

    RefreshLayerFile();
}

void MainWindow::RefreshLayerFile()
{
    CurrentSelectedItem = nullptr;
    LayerScene->clear(); //remove all objects here

    ui->GroundTypeComboBox->setEnabled(false);
    ui->ItemTypeComboBox->setEnabled(false);
    ui->EnemyCombobox->setEnabled(false);

    for(int i = this->LFH->LFI->count() - 1; i >= 0; i --)
    {
        this->LFH->LFI->at(i)->clear(); //remove all individual objects
    }
    this->LFH->LFI->clear(); //clear out entire 2D array

    QList<LayerfileHandler::SubSection*>* buf = this->LFH->LoadedFile->Sections->at(LayerLevel)->SubSections;

    std::sort(buf->begin(), buf->end(), [=](LayerfileHandler::SubSection* a, LayerfileHandler::SubSection* b)->bool{
        return (a->HeaderbyteTwo > b->HeaderbyteTwo) ? true : (a->HeaderbyteOne > b->HeaderbyteOne) ? true : false;
    });

    std::reverse(buf->begin(), buf->end());

    for(int y = 0; y < this->LFH->LoadedFile->Sections->at(LayerLevel)->AmmSubsections; y++)
    {
        for(int xx = 0; xx < 8; xx++)
        {
            QList<LayerFileItem*>* buftwo = new QList<LayerFileItem*>();
            for(int x = 0; x < this->LFH->LoadedFile->Sections->at(LayerLevel)->AmmPartsInSubsections; x++)
            {
                for(int w = 0; w < 8; w++)
                {
                    LayerFileItem* L = new LayerFileItem();
                    L->Layer = 0;
                    L->Subsection = y * this->LFH->LoadedFile->Sections->at(LayerLevel)->AmmPartsInSubsections + x;
                    L->DataItem = buf->at(L->Subsection)->Data->at(xx * 8 + w);
                    L->DataPos = xx * 8 + w;
                    L->setRect(0,0,SquareSize,SquareSize); //100x100 square
                    L->RefreshVisual();
                    QObject::connect(L, &LayerFileItem::clicked, [=](LayerFileItem *item, int DataItem) {
                        //okay we got our data here now yippee
                        userInput = false;
                        ui->GroundTypeComboBox->setEnabled(true);
                        ui->ItemTypeComboBox->setEnabled(true);                        
                        ui->GroundTypeComboBox->setCurrentIndex(DataItem & 0b00001111);
                        ui->ItemTypeComboBox->setCurrentIndex((DataItem >> 4) & 0b00001111);
                        ui->EnemyCombobox->setCurrentIndex((DataItem >> 8) & 0b00001111);
                        ui->EnemyCombobox->setEnabled(true);                        
                        ui->LayerfileEditorValueBox->setValue(DataItem);
                        CurrentSelectedItem = item;
                        ui->spinBoxXVal->setValue(item->X);
                        ui->spinBoxYVal->setValue(item->Y);
                        userInput = true;
                    });
                    QObject::connect(L, &LayerFileItem::RightClicked, [=](LayerFileItem *item){
                        if(CurrentSelectedItem != nullptr)
                        {
                            userInput = false;
                            item->DataItem = CurrentSelectedItem->DataItem;
                            this->LFH->LoadedFile->Sections->at(LayerLevel)->SubSections->at(item->Subsection)->Data->replace(item->DataPos, (short unsigned int)item->DataItem);
                            userInput = true;
                            item->RefreshVisual();
                        }
                    });
                    buftwo->append(L);
                    L->Y = y * 8 + xx;
                    L->X = x * 8 + w;
                }
            }
            this->LFH->LFI->append(buftwo);
        }
    }

    for(int y = 0; y < this->LFH->LFI->count(); y++)
    {
        for(int x = 0; x < this->LFH->LFI->at(y)->count(); x++)
        {
            this->LFH->LFI->at(y)->at(x)->setPos(x * SquareSize, y * SquareSize);
            LayerScene->addItem(this->LFH->LFI->at(y)->at(x));
        }
    }    
}


void MainWindow::on_Zoomslider_sliderMoved(int position)
{

}


void MainWindow::on_Zoomslider_valueChanged(int value)
{
    SquareSize = value;
    for(int y = 0; y < this->LFH->LFI->count(); y++)
    {
        for(int x = 0; x < this->LFH->LFI->at(y)->count(); x++)
        {
            this->LFH->LFI->at(y)->at(x)->setRect(0, 0, SquareSize, SquareSize);
            this->LFH->LFI->at(y)->at(x)->setPos(x * SquareSize, y * SquareSize);
        }
    }
}


void MainWindow::on_LayerSelectSlider_valueChanged(int value)
{
    LayerLevel = value;
    ui->HeightOffsetPerLayerSpinBox->setValue(LFH->LoadedFile->Sections->at(LayerLevel)->HeightOffset);
    ui->HeightOffsetPerLayerSpinBox->setEnabled(true);
    RefreshLayerFile();
}


void MainWindow::on_SaveBinButton_clicked()
{
    // Convert to binary -> download
    QByteArray OutFile;
    qint32 Length = 0;
    QDataStream dataStream(&OutFile, QIODevice::WriteOnly);
    dataStream.setByteOrder(QDataStream::BigEndian);

    dataStream << LFH->LoadedFile->Header;
    Length += 4;
    dataStream << LFH->LoadedFile->DeathOffset;
    Length++;

    for (int i = 0; i < LFH->LoadedFile->Sections->length(); i++) {
        dataStream << LFH->LoadedFile->Sections->at(i)->AmmPartsInSubsections;
        Length++;
        dataStream << LFH->LoadedFile->Sections->at(i)->AmmSubsections;
        Length++;
        dataStream << LFH->LoadedFile->Sections->at(i)->HeightOffset;
        Length++;
        for (int j = 0; j < LFH->LoadedFile->Sections->at(i)->SubSections->length(); j++) {
            dataStream << LFH->LoadedFile->Sections->at(i)->SubSections->at(j)->HeaderbyteOne;
            Length++;
            dataStream << LFH->LoadedFile->Sections->at(i)->SubSections->at(j)->HeaderbyteTwo;
            Length++;
            for (int k = 0; k < LFH->LoadedFile->Sections->at(i)->SubSections->at(j)->Data->length(); k++) {
                dataStream << LFH->LoadedFile->Sections->at(i)->SubSections->at(j)->Data->at(k);
                Length += 2;
            }
        }
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save binary", "col.bin", "BIN files (*.bin)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(OutFile.constData(), Length);
            file.close();
        }
    }
}


void MainWindow::on_GroundTypeComboBox_currentIndexChanged(int index)
{
    if (!userInput)
        return;

    if(CurrentSelectedItem != nullptr)
    {
        CurrentSelectedItem->DataItem &= 0b11111111111110000;
        CurrentSelectedItem->DataItem |= (0b00001111 & index);
        CurrentSelectedItem->RefreshVisual();
        this->LFH->LoadedFile->Sections->at(LayerLevel)->SubSections->at(CurrentSelectedItem->Subsection)->Data->replace(CurrentSelectedItem->DataPos, (short unsigned int)CurrentSelectedItem->DataItem);
    }
}


void MainWindow::on_ItemTypeComboBox_currentIndexChanged(int index)
{
    if (!userInput)
        return;

    if(CurrentSelectedItem != nullptr)
    {
        CurrentSelectedItem->DataItem &= 0b11111111100001111;
        CurrentSelectedItem->DataItem |= (0b11110000 & (index << 4));
        CurrentSelectedItem->RefreshVisual();
        this->LFH->LoadedFile->Sections->at(LayerLevel)->SubSections->at(CurrentSelectedItem->Subsection)->Data->replace(CurrentSelectedItem->DataPos, (short unsigned int)CurrentSelectedItem->DataItem);
    }
}

void MainWindow::on_ClearfileButton_clicked()
{
    for (int i = 0; i < LFH->LoadedFile->Sections->length(); i++) {
        for (int j = 0; j < LFH->LoadedFile->Sections->at(i)->SubSections->length(); j++) {
            for (int k = 0; k < LFH->LoadedFile->Sections->at(i)->SubSections->at(j)->Data->length(); k++) {
                LFH->LoadedFile->Sections->at(i)->SubSections->at(j)->Data->replace(k, 0x10);
            }
        }
    }

    RefreshLayerFile();
}


void MainWindow::on_EnemyCombobox_currentIndexChanged(int index)
{
    if (!userInput)
        return;

    if(CurrentSelectedItem != nullptr)
    {
        CurrentSelectedItem->DataItem &= 0b11111000011111111;
        CurrentSelectedItem->DataItem |=  (index << 8);
        this->LFH->LoadedFile->Sections->at(LayerLevel)->SubSections->at(CurrentSelectedItem->Subsection)->Data->replace(CurrentSelectedItem->DataPos, (short unsigned int)CurrentSelectedItem->DataItem);
        CurrentSelectedItem->RefreshVisual();
    }
}


void MainWindow::on_BinLoader_clicked()
{
    QString Wantedfile = QFileDialog::getOpenFileName(this, tr("Open file"));
    QFile file(Wantedfile);

    if(file.open(QIODevice::ReadOnly))
    {
        this->LFH->LoadLayerFile(file.readAll());
        this->ui->LayerSelectSlider->setValue(0);
        this->ui->LayerSelectSlider->setMaximum(this->LFH->LoadedFile->Sections->count() - 1);
    }

    RefreshLayerFile();
}


void MainWindow::on_HeightOffsetPerLayerSpinBox_valueChanged(int arg1)
{
    this->LFH->LoadedFile->Sections->at(LayerLevel)->HeightOffset = arg1;
}

int GeneratePlane(QString objectName, float x, float y, float z, QColor color, QString colorName, int index)
{
    qDebug() << x << " " << z << endl;

    QFile objFile(objectName + ".obj");
    if (objFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream out(&objFile);

        out << "v " << x << " " << y << " " << z << endl;
        out << "v " << x+EXPORTSQUARESIZE << " " << y << " " << z << endl;
        out << "v " << x+EXPORTSQUARESIZE << " " << y << " " << z + EXPORTSQUARESIZE << endl;
        out << "v " << x << " " << y << " " << z + EXPORTSQUARESIZE << endl;

        out << "usemtl " << colorName << endl;
        out << "f " << index+3 << " " << index+2 << " " << index+1 << " " << index << endl;
        objFile.close();
    }

    QFile mtlFile(objectName + ".mtl");
    if (mtlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&mtlFile);
        QString line;
        bool colorNameExists = false;
        while (in.readLineInto(&line)) {
            if (line.startsWith("newmtl " + colorName)) {
                colorNameExists = true;
                break;
            }
        }
        mtlFile.close();

        if (!colorNameExists) {
            if (mtlFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
                QTextStream out(&mtlFile);
                out << "newmtl " << colorName << endl;
                out << "Kd " << color.redF() << " " << color.greenF() << " " << color.blueF() << endl;
                mtlFile.close();
            }
        }
    }

    return index + 4;
}


void MainWindow::GenerateObj(QString objectName)
{
    QFile objFile(objectName + ".obj");
    if (objFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&objFile);
        out << "mtllib " << objectName + ".mtl" << endl;

        objFile.close();
    }

    QFile mtlFile(objectName + ".mtl");
    if (mtlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&mtlFile);
        out << "";
        mtlFile.close();
    }

    int index = 1;
    float HeightOffset = 0;
    QVector3D SpawnPos(0,0,0);

    for(int Y = 0; Y < this->LFH->LoadedFile->Sections->size(); Y++)
    {
        QList<LayerfileHandler::SubSection*>* buf = this->LFH->LoadedFile->Sections->at(Y)->SubSections;

        std::sort(buf->begin(), buf->end(), [=](LayerfileHandler::SubSection* a, LayerfileHandler::SubSection* b)->bool{
            return (a->HeaderbyteTwo > b->HeaderbyteTwo) ? true : (a->HeaderbyteOne > b->HeaderbyteOne) ? true : false;
        });

        std::reverse(buf->begin(), buf->end());

        for(int y = 0; y < buf->size(); y++)
        {
            for(int z = 0; z  < 8; z++)
            {
                for(int x = 0; x < 8; x++)
                {
                    LayerfileHandler::SubSection ss = *(buf->at(y));
                    QVector3D BufOffset = SpawnPos + QVector3D(x, 0, z) + QVector3D((ss.HeaderbyteOne * 8), 0, (ss.HeaderbyteTwo * 8));
                    uint val = ss.Data->at(x + z * 8);
                    bool denied = false;

                    QColor MyCol;
                    switch(val & 0b00001111)
                    {
                        case 0:
                        {
                            //air
                            denied = true;
                            break;
                        }
                        case 1:
                        {
                            //floor
                            QRgb col = 0xFFA500;
                            QColor oreng(col);

                            MyCol = oreng;
                            break;
                        }
                        case 2:
                        {
                            //TransitionSquare
                            QColor oreng(Qt::green);
                            MyCol = oreng;
                            break;
                        }
                        case 3:
                        {
                            //ramp down
                            MyCol = (Qt::red);
                            break;
                        }
                        case 4:
                        {
                            //ramp left
                            MyCol = (Qt::red);
                            break;
                        }
                        case 5:
                        {
                            //ramp right
                            MyCol = (Qt::red);
                            break;
                        }
                        case 6:
                        {
                            //ramp up
                            MyCol = (Qt::red);
                            break;
                        }
                        case 9:
                        {
                            MyCol = (Qt::darkRed);
                            break;
                        }
                        case 0xF:
                {
                    MyCol = (Qt::gray);
                    break;
                    }

                    }
                    int offsetter = EXPORTSQUARESIZE / 2;

                    if(!denied)
                    {
                        index = GeneratePlane(objectName, BufOffset.x() * EXPORTSQUARESIZE - offsetter, BufOffset.y() * EXPORTSQUARESIZE, BufOffset.z() * EXPORTSQUARESIZE - offsetter, MyCol, MyCol.name(), index);
                    }
                }
            }
        }

        SpawnPos.setY(SpawnPos.y() + (this->LFH->LoadedFile->Sections->at(Y)->HeightOffset));
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    QFileDialog fileDialog;
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDefaultSuffix("obj");
    if (fileDialog.exec() == QDialog::Accepted) {
        QString objectName = fileDialog.selectedFiles().first();
        objectName.chop(4); // remove ".obj" from the end of the string
        GenerateObj(objectName);
    }
}
