#ifndef PREFERENCESMANAGER_H
#define PREFERENCESMANAGER_H

#include <QString>

class PreferencesManager
{

public:
    PreferencesManager(QString PrefPath);

    QString ROMPath;
    void WritePreferences();

private:
    QString PreferencesPath;
};

#endif // PREFERENCESMANAGER_H
