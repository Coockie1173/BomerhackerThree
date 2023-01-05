#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "preferencesmanager.h"
#include "layerfilehandler.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString ToolsFolder;
    PreferencesManager* Preferences;
    LayerfileHandler* LFH;

private slots:
    void on_SetROMButton_clicked();

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void RefreshLayerFile();

    void on_Zoomslider_sliderMoved(int position);

    void on_Zoomslider_valueChanged(int value);

    void on_LayerSelectSlider_valueChanged(int value);

    void on_SaveBinButton_clicked();

    void on_GroundTypeComboBox_currentIndexChanged(int index);

    void on_ItemTypeComboBox_currentIndexChanged(int index);

    void on_pushButton_3_clicked();

    void on_ClearfileButton_clicked();

    void on_EnemyCombobox_currentIndexChanged(int index);

    void on_BinLoader_clicked();

    void on_HeightOffsetPerLayerSpinBox_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;

    LayerFileItem* CurrentSelectedItem = nullptr;

    const QString FloorIdentifier[0x10] = {"Air 0", "StandardFloor 1", "Action Square 2", "BackwardsRamp 3", "ForwardsRamp 4", "RightwardsRamp 5", "LeftwardsRamp 6", "Pusherback 7","Pusherfront 8","Deathplane 9","Unk1 0xA", "IceFloorToggler 0xB", "Unk3 0xC", "Unk4 0xD", "Unk5 0xE","Wall 0xF"};
    const QString ItemIdentifier[0x10] = {"0", "1", "2", "3", "4", "5", "6", "7","8","9","10", "11", "12", "13", "14","15"};

    bool userInput = false;

    void GenerateObj(QString objectName);
};
#endif // MAINWINDOW_H
