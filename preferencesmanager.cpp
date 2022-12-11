#include "preferencesmanager.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>

PreferencesManager::PreferencesManager(QString PrefPath)
{
    PreferencesPath = PrefPath;

    QFile file = QFile(PreferencesPath);

    if(QFileInfo::exists(PrefPath))
    {
        qDebug() << "preferences found... loading...\n";
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        QTextStream in(&file);

        ROMPath = in.readLine();

        file.close();
    }
    else
    {
        qDebug() << "preferences not found... loading default settings...\n";
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        file.write("");
        qDebug()<<"file created\n";
        file.close();

    }
}

void PreferencesManager::WritePreferences()
{
    QFile file = QFile(PreferencesPath);
    file.remove();
    file.open(QIODevice::ReadWrite | QIODevice::Text);
    QTextStream out(&file);

    out << ROMPath;

    file.close();
    qDebug()<<"Preferences written\n";
}
