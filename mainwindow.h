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

private:
    Ui::MainWindow *ui;

    LayerFileItem* CurrentSelectedItem = nullptr;

    const QString ItemIdentifiers[0x10] = {"Air", "StandardFloor", "TransitionSquare", "BackwardsRamp", "ForwardsRamp", "RightwardsRamp", "LeftwardsRamp", "Pusherback","Pusherfront","Deathplane","Unk1", "IceFloorToggler", "Unk3", "Unk4", "Unk5","Wall"};
};
#endif // MAINWINDOW_H
