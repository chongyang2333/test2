#ifndef SETTINGINI_H
#define SETTINGINI_H


#include <QString>
#include <QMutex>

#define CURRENT_TAB_INDEX "cur_index"

class SettingINI
{
public:
    static SettingINI *getInstance() {
        return mSetingIni;
    }
    bool save(QString name, QString value);
    bool save(QString name, int value);
    QString getString(QString name);
    int get(QString name);

private:
    SettingINI();
    QByteArray readSettingIni();
    bool writeSettingIni(QByteArray data);

private:
    QMutex mutex;
    static SettingINI *mSetingIni;
    static QString path;
};

#endif // SETTINGINI_H
