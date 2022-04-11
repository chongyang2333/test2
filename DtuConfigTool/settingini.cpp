#include "settingini.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>

QString SettingINI::path = "settings.ini";
SettingINI *SettingINI::mSetingIni = new SettingINI;

SettingINI::SettingINI()
{

}

bool SettingINI::save(QString name, int value)
{
    QByteArray data = readSettingIni();

    QJsonParseError err;
    QJsonObject root;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "Setting parse error, " << err.errorString();
        goto saveItem;
    }
    root = doc.object();
    if (root.contains(name) && root.value(name).toInt() == value) {
        return true;
    }

saveItem:
    root.insert(name, value); 
    doc.setObject(root);
    data = doc.toJson(QJsonDocument::Indented);
    //qDebug() << data;
    writeSettingIni(data);

    return true;
}

bool SettingINI::save(QString name, QString value)
{
    QByteArray data = readSettingIni();
    QJsonParseError err;
    QJsonObject root;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "Setting parse error, " << err.errorString();
        goto saveItem;
    }
    root = doc.object();
    if (root.contains(name) && root.value(name).toString() == value) {
        return true;
    }

saveItem:
    root.insert(name, value);
    doc.setObject(root);
    data = doc.toJson(QJsonDocument::Indented);
    //qDebug() << data;
    writeSettingIni(data);
    return true;
}

QString SettingINI::getString(QString name)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "open " << path << " failed";
        return "";
    }
    QByteArray data = file.readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "Setting parse error, " << err.errorString();
        return "";
    }
    QJsonObject root = doc.object();
    if (root.contains(name) && root.value(name).isString()) {
        file.close();
        return root.value(name).toString();
    }

    file.close();
    return "";
}

int SettingINI::get(QString name)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "open " << path << " failed";
        return 0;
    }
    QByteArray data = file.readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "Setting parse error, " << err.errorString();
        return 0;
    }
    QJsonObject root = doc.object();
    if (root.contains(name) && root.value(name).isDouble()) {
        file.close();
        return root.value(name).toInt();
    }

    file.close();
    return 0;
}


int SettingINI::getDtuTypeIndex(QString name)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        qDebug() << "open " << path << " failed";
        return 0;
    }
    QByteArray data = file.readAll();
//    qDebug() << "read all "<<data;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "Setting parse error, " << err.errorString();
        return 0;
    }
    QJsonObject root = doc.object();
    if (root.contains(name) && root.value(name).isDouble()) {
        file.close();
//        qDebug() << "Setting parse read succceed, " << root.value(name).toInt();
        return root.value(name).toInt();
    }

    file.close();
    return 0;
}


/**
 * @brief SettingINI::readSettingIni 把设置文件内容读取到QByteArray中
 * @return
 */
QByteArray SettingINI::readSettingIni()
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "open " << path << " failed";
        return "";
    }
    QByteArray data = file.readAll();
    file.close();
    return data;
}

/**
 * @brief SettingINI::writeSettingIni 会清除设置文件中的所有内容
 * @param data 写入的数据
 * @return
 */
bool SettingINI::writeSettingIni(QByteArray data)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        qDebug() << "open " << path << " failed";
        return false;
    }
    file.write(data);
    file.close();
    return true;
}
