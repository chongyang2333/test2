#include "pdhttpapi.h"

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>

PdHttpApi *PdHttpApi::pdHttpApi = nullptr;

PdHttpApi::PdHttpApi(QObject *parent):PdHttpClient(parent, false)
{
    maxRetryTimes = 5;
    // qDebug()<<"PdHttpApi pid = "<<QThread::currentThread();
//    getAccessToken();
}

int PdHttpApi::getAccessToken()
{
    QJsonObject json;
    json.insert("client_id", CLIENT_ID);
    json.insert("client_secret", CLIENT_SECRET);
    json.insert("grant_type", GRANT_TYPE);
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray byteData = doc.toJson(QJsonDocument::Compact);

    QString url = QString(BASE_URL) + QString(AUTH_URL);

    QNetworkReply *reply = httpClientPost(url, nullptr, byteData);

    if (reply == nullptr) {
        qDebug() << "httpClientPost failed";
        return -1;
    }

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200) {
        qWarning() << "getAccessToken failed, status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return -1;
    }

    QByteArray replyData = reply->readAll();
    qDebug() << replyData;
    QJsonParseError jsonError;

    QJsonDocument rdoc = QJsonDocument::fromJson(replyData, &jsonError);
    if (!rdoc.isNull() && jsonError.error == QJsonParseError::NoError) {
        if (rdoc.isObject()) {
            QJsonObject obj = rdoc.object();
            if (obj.contains("access_token")) {
                accessToken = obj.value("access_token").toString();
            }
            if (obj.contains("expires_in")) {
                expiresIn = obj.value("expires_in").toInt();
            }
            if (obj.contains("token_type")) {
                tokenType = obj.value("token_type").toString();
            }
        }
    }
    return 0;
}


PdDeviceTcpRsgisterInfo *PdHttpApi::getTcpRegisterInfo(QString pid)
{
    qDebug()<<"getTcpRegisterInfo : pid = "<< pid;

    QString url = QString(HTTP_BASE_URL) + QString(HTTP_GET_DTUCFG_DOMAIN_SUFFIX_1) + \
                  QString(pid) + QString(HTTP_GET_DTUCFG_DOMAIN_SUFFIX_2);

    qDebug() <<"getTcpRegisterInfo : url = " << url;
    QNetworkReply *reply = httpClientGetData(url);
    if (reply == nullptr) 
    {
        qDebug() << "getTcpRegisterInfo->get : failed";
        return nullptr;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 401) 
    {
        if (retryTimes >= maxRetryTimes) 
        {
            qDebug() << "getTcpRegisterInfo->get : failed" << retryTimes << " times, but failed";
            return nullptr;
        }
        retryTimes++;
        return getTcpRegisterInfo(pid); //重新尝试获取设备信息
    }
    else if (statusCode != 200) 
    {
        qWarning() << "getTcpRegisterInfo->get： failed,status code =" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return nullptr;
    }   

    qDebug() << "getTcpRegisterInfo->get succeed,statusCode = " << statusCode;
    QByteArray replyData = reply->readAll();
    qDebug() << replyData;   
    QJsonParseError jsonError;

    QJsonDocument rdoc = QJsonDocument::fromJson(replyData, &jsonError);
    if (!rdoc.isNull() && jsonError.error == QJsonParseError::NoError) 
    {
        if (rdoc.isObject()) 
        {
            QJsonObject obj = rdoc.object();
            if (obj.contains("code")) 
            {
                qint64 code = QString::number(obj.value("code").toDouble(), 'f', 0).toLongLong();
                if ((code == 200  || code == 0) && obj.contains("data")) 
                {
                    PdDeviceTcpRsgisterInfo *info = new PdDeviceTcpRsgisterInfo;
                    QJsonObject infoObj = obj.value("data").toObject();
                    if (infoObj.contains("host")) 
                    {
                        info->tcpHostAdress = infoObj.value("host").toString().toLocal8Bit();
//                        QString tmp = infoObj.value("host").toString();
//                        info->tcpHostPort     = tmp.right(tmp.size()-tmp.lastIndexOf(":")-1).toLocal8Bit();
//                        info->tcpHostAdress   = tmp.left(tmp.lastIndexOf(":")).toLocal8Bit();

                    }
                    if (infoObj.contains("port"))
                    {
                        info->tcpHostPort = QString::number(infoObj.value("port").toDouble()).toLocal8Bit();
                    }
                    if (infoObj.contains("pid")) 
                    {
                        info->pid = infoObj.value("pid").toString().toLocal8Bit();
                    }
                    if (infoObj.contains("secret")) 
                    {
                        info->tcpHostSecret = infoObj.value("secret").toString().toLocal8Bit();
                    }
                   qDebug()<<"get tcp regester succeed";
                   qDebug()<<"host = "     << info->tcpHostAdress;
                   qDebug()<<"pid = "      << info->pid ;
                   qDebug()<<"serect = "   << info->tcpHostSecret;
                   qDebug()<<"tcpHostPort = "   << info->tcpHostPort;
                    return info;
                }
            }
        }
    }
    return nullptr;

}

PdDeviceRegisterInfo *PdHttpApi::getRegisterInfo(QString pid)
{
    qDebug()<<"getRegisterInfo: "<< pid;
    if (accessToken.isEmpty()) {
        if(getAccessToken() < 0) {
            qWarning()<< "getAccessToken failed";
            return nullptr;
        }
    }

    QJsonObject json;
    json.insert("pid", pid);
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray byteData = doc.toJson(QJsonDocument::Compact);
    qDebug()<<byteData;

    QString url = QString(BASE_URL) + QString(GET_REGISTER_INFO);

    QNetworkReply *reply = httpClientPost(url, accessToken, byteData);
    if (reply == nullptr) {
        qDebug() << "httpClientPost failed";
        return nullptr;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 401) {
        if (retryTimes >= maxRetryTimes) {
            qDebug() << "Get access token " << retryTimes << " times, but failed";
            return nullptr;
        }
        retryTimes++;
        if(getAccessToken() < 0) {
            qWarning()<< "getAccessToken failed";
            return nullptr;
        }
        return getRegisterInfo(pid); //重新尝试获取设备信息
    }else if (statusCode != 200) {
        qWarning() << "getRegisterInfo failed, status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return nullptr;
    }

    QByteArray replyData = reply->readAll();
    qDebug() << replyData;
    QJsonParseError jsonError;

    QJsonDocument rdoc = QJsonDocument::fromJson(replyData, &jsonError);
    if (!rdoc.isNull() && jsonError.error == QJsonParseError::NoError) {
        if (rdoc.isObject()) {
            QJsonObject obj = rdoc.object();
            if (obj.contains("code")) {
                qint64 code = QString::number(obj.value("code").toDouble(), 'f', 0).toLongLong();
                if ((code == 200  || code == 0) && obj.contains("data")) {
                    PdDeviceRegisterInfo *info = new PdDeviceRegisterInfo;
                    QJsonObject infoObj = obj.value("data").toObject();
                    if (infoObj.contains("product_key")) {
                        info->productKey = infoObj.value("product_key").toString().toLocal8Bit();
                    }
                    if (infoObj.contains("device_name")) {
                        info->deviceName = infoObj.value("device_name").toString().toLocal8Bit();
                    }
                    if (infoObj.contains("device_secret")) {
                        info->deviceSecret = infoObj.value("device_secret").toString().toLocal8Bit();
                    }
                    if (infoObj.contains("isp")) {
                        info->isp = infoObj.value("isp").toString().toLocal8Bit();
                    }

                    return info;
                }
            }
        }
    }

    return nullptr;
}

bool PdHttpApi::sendSelfCheckStatusToMes(QString pid)
{
    qDebug()<<"sendSelfCheckStatusToMes： pid = "<< pid;


    QJsonObject json;
    json.insert("status",1);
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray byteData = doc.toJson(QJsonDocument::Compact);

    QString url = QString(HTTP_BASE_URL) + QString(HTTP_PATCH_DOMAIN_SUFFIX_1) + \
                  QString(pid) + QString(HTTP_PATCH_DOMAIN_SUFFIX_2);
    qDebug() <<"sendSelfCheckStatusToMes： url = " << url;
    QNetworkReply *reply = httpClientPatchData(url,byteData);
    if (reply == nullptr) 
    {
        qDebug() << "sendSelfCheckStatusToMes->patch： failed";
        return -1;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 200) 
    {
        qWarning() << "sendSelfCheckStatusToMes->patch:failed, status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return false;
    }

    QByteArray replyData = reply->readAll();
    qDebug() << replyData;
    QJsonParseError jsonError;

    QJsonDocument rdoc = QJsonDocument::fromJson(replyData, &jsonError);
    if (!rdoc.isNull() && jsonError.error == QJsonParseError::NoError) 
    {
        if (rdoc.isObject()) 
        {
            QJsonObject obj = rdoc.object();
            if (obj.contains("code")) 
            {
                qint64 code = QString::number(obj.value("code").toDouble(), 'f', 0).toLongLong();
                qDebug()<< "code = " << code;
                if (code == 200  || code == 0 ) 
                {
                    return true;
                }
            }
        }
    }

    return false;
}


bool PdHttpApi::active(QString pid)
{
    qDebug()<<"active: "<< pid;
    if (accessToken.isEmpty()) {
        if(getAccessToken() < 0) {
            qWarning()<< "getAccessToken failed";
            return false;
        }
    }

    QJsonObject json;
    json.insert("pid", pid);
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray byteData = doc.toJson(QJsonDocument::Compact);

    QString url = QString(BASE_URL) + QString(ACTIVE_URL);

    QNetworkReply *reply = httpClientPost(url, accessToken, byteData);
    if (reply == nullptr) {
        qDebug() << "httpClientPost failed";
        return false;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 401) {
        if (retryTimes >= maxRetryTimes) {
            qDebug() << "Get access token " << retryTimes << " times, but failed";
            return false;
        }
        retryTimes++;
        if(getAccessToken() < 0) {
            qWarning()<< "getAccessToken failed";
            return false;
        }
        return active(pid); //重新尝试
    }else if (statusCode != 200) {
        qWarning() << "active failed, status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return false;
    }

    QByteArray replyData = reply->readAll();
    qDebug() << replyData;
    QJsonParseError jsonError;

    QJsonDocument rdoc = QJsonDocument::fromJson(replyData, &jsonError);
    if (!rdoc.isNull() && jsonError.error == QJsonParseError::NoError) {
        if (rdoc.isObject()) {
            QJsonObject obj = rdoc.object();
            if (obj.contains("code")) {
                qint64 code = QString::number(obj.value("code").toDouble(), 'f', 0).toLongLong();
                qDebug()<< "code = " << code;
                if (code == 200  || code == 0 ) {
                    return true;
                }
            }
        }
    }

    return false;
}

int PdHttpApi::getTcpClientSelfCheckResult(QString pid)
{
    qDebug()<<"getTcpClientSelfCheckResult : pid = "<< pid;

    QString url = QString(HTTP_BASE_URL) + QString(HTTP_GET_INSPECT_STATUS_DOMAIN_SUFFIX_1) + \
                  QString(pid) + QString(HTTP_GET_INSPECT_STATUS_DOMAIN_SUFFIX_2);

    qDebug() <<"getTcpClientSelfCheckResult : url = " << url;
    QNetworkReply *reply = httpClientGetData(url);
    if (reply == nullptr) 
    {
        qDebug() << "getTcpClientSelfCheckResult->get : failed";
        return -1;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 401) 
    {
        if (retryTimes >= maxRetryTimes) 
        {
            qDebug() << "getTcpClientSelfCheckResult->get:failed" << retryTimes << " times, but failed";
            return -1;
        }
        retryTimes++;
        return getTcpClientSelfCheckResult(pid); //重新尝试
    }
    else if (statusCode != 200) 
    {
        qWarning() << "getTcpClientSelfCheckResult->get:failed,status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return -1;
    }

    qDebug() << "getTcpClientSelfCheckResult->get:succeed,statusCode = " << statusCode;
    QByteArray replyData = reply->readAll();
    qDebug() << replyData;
    QJsonParseError jsonError;

    QJsonDocument rdoc = QJsonDocument::fromJson(replyData, &jsonError);
    if (!rdoc.isNull() && jsonError.error == QJsonParseError::NoError) 
    {
        if (rdoc.isObject()) 
        {
            QJsonObject obj = rdoc.object();
            if (obj.contains("code")) 
            {
                qint64 code = QString::number(obj.value("code").toDouble(), 'f', 0).toLongLong();
                if ((code == 200  || code == 0) && obj.contains("data")) 
                {
                    QJsonObject dataObj = obj.value("data").toObject();
                    if (dataObj.contains("status")) 
                    {
                        qDebug()<<"SelfCheckResult succeed,status:"<< dataObj.value("status").toInt();    
                        return dataObj.value("status").toInt();
                    }
                }
            }
        }
    }

    return -1;    

}

int PdHttpApi::getSelfCheckResult(QString pid)
{
    qDebug()<<"getSelfCheckResult: "<< pid;
    if (accessToken.isEmpty()) {
        if(getAccessToken() < 0) {
            qWarning()<< "getAccessToken failed";
            return -1;
        }
    }

    QJsonObject json;
    json.insert("pid", pid);
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray byteData = doc.toJson(QJsonDocument::Compact);

    QString url = QString(BASE_URL) + QString(SELF_CHECK_RESULT);

    QNetworkReply *reply = httpClientPost(url, accessToken, byteData);
    if (reply == nullptr) {
        qDebug() << "httpClientPost failed";
        return -1;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 401) {
        if (retryTimes >= maxRetryTimes) {
            qDebug() << "Get access token " << retryTimes << " times, but failed";
            return -1;
        }
        retryTimes++;
        if(getAccessToken() < 0) {
            qWarning()<< "getAccessToken failed";
            return -1;
        }
        return getSelfCheckResult(pid); //重新尝试
    }else if (statusCode != 200) {
        qWarning() << "getRegisterInfo failed, status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return -1;
    }

    QByteArray replyData = reply->readAll();
    qDebug() << replyData;
    QJsonParseError jsonError;

    QJsonDocument rdoc = QJsonDocument::fromJson(replyData, &jsonError);
    if (!rdoc.isNull() && jsonError.error == QJsonParseError::NoError) {
        if (rdoc.isObject()) {
            QJsonObject obj = rdoc.object();
            if (obj.contains("code")) {
                qint64 code = QString::number(obj.value("code").toDouble(), 'f', 0).toLongLong();
                if ((code == 200  || code == 0) && obj.contains("data")) {
                    QJsonObject dataObj = obj.value("data").toObject();
                    if (dataObj.contains("status")) {
                        return dataObj.value("status").toInt();
                    }
                }
            }
        }
    }

    return -1;
}

