#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QProcess>
#include <QFileInfo>
#include <algorithm>    // std::sort
#include "layerfileitem.h"

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
        ui->GroundTypeComboBox->addItem(ItemIdentifiers[i]);
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
                    buftwo->append(L);
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
    qDebug() << value;
    RefreshLayerFile();
}

