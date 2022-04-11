#include "pdhttpapi.h"

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonDocument>

PdHttpApi *PdHttpApi::pdHttpApi = nullptr;

PdHttpApi::PdHttpApi(QObject *parent):PdHttpClient(parent, false)
{
    maxRetryTimes = 1;
    retryTimes = 0;
    // qDebug()<<"PdHttpApi pid = "<<QThread::currentThread();
    getAccessToken();
}

int PdHttpApi::getAccessToken()
{
    QJsonObject json;
    json.insert("client_id", HTTP_CLIENT_ID);
    json.insert("secret", HTTP_CLIENT_SECRET);
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray byteData = doc.toJson(QJsonDocument::Compact);

    QString url = QString(HTTP_BASE_URL) + QString(HTTP_POST_TOKEN_DOMAIN_SUFFIX);

    QNetworkReply *reply = httpClientPost(url, nullptr, byteData);

    if (reply == nullptr) 
    {
        qDebug() << "httpClientPost failed";
        return -1;
    }

    if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() != 200)
    {
        qWarning() << "getAccessToken failed, status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return -1;
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
                if ((code == 200  || code == 0) && obj.contains("data")) 
                {
                    QJsonObject infoObj = obj.value("data").toObject();
                    if (infoObj.contains("access_token"))
                    {
                        accessToken = infoObj.value("access_token").toString();
                    }
                    if (infoObj.contains("expires_in")) 
                    {
                        expiresIn = infoObj.value("expires_in").toInt();
                    }
                }
            }            
        }
    }
    return 0;
}


PdDeviceTcpRsgisterInfo *PdHttpApi::getTcpRegisterInfo(QString pid)
{
    qDebug()<<"getTcpRegisterInfo : pid = "<< pid;
    if(accessToken.isEmpty())
    {
        if(getAccessToken() < 0)
        {
            qWarning()<< "getAccessToken failed";
            return nullptr;
        }
    }

    QString url = QString(HTTP_BASE_URL) + QString(HTTP_GET_DTUCFG_DOMAIN_SUFFIX_1) + \
                  QString(pid) + QString(HTTP_GET_DTUCFG_DOMAIN_SUFFIX_2);

    qDebug() <<"getTcpRegisterInfo : url = " << url << ",accessToken :"<<accessToken;
    QNetworkReply *reply = httpClientGet(url,accessToken);
    if (reply == nullptr) 
    {
        qDebug() << "getTcpRegisterInfo->get : failed";
        return nullptr;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(statusCode == 401) 
    {
        qDebug() << "getTcpRegisterInfo->get : failed" << retryTimes << " times, but failed";
        if(retryTimes >= maxRetryTimes) 
        {
            qDebug() << "getTcpRegisterInfo->get : failed" << retryTimes << " times, but failed";
            retryTimes = 0;
            return nullptr;
        }
        retryTimes++;
        if(getAccessToken() < 0)
        {
            qWarning()<< "getAccessToken failed";
            return nullptr;
        }        
        return getTcpRegisterInfo(pid); //重新尝试获取设备信息
    }
    else if (statusCode != 200) 
    {
        qWarning() << "getTcpRegisterInfo->get: failed,status code =" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
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
int PdHttpApi::getdishwasherSignal(QString pid)
{
    qDebug()<<"getdishwasherSignal : pid = "<< pid;
    if(accessToken.isEmpty())
    {
        if(getAccessToken() < 0)
        {
            qWarning()<< "getdishwasherSignal getAccessToken failed";
            return -1;
        }
    }

    QString url = QString(HTTP_BASE_URL) + QString(HTTP_GET_DISHWASHER_SIGNAL_DOMAIN_SUFFIX_1) + \
                  QString(pid) + QString(HTTP_GET_DISHWASHER_SIGNAL_DOMAIN_SUFFIX_2);

    qDebug() <<"getdishwasherSignal : url = " << url;
    QNetworkReply *reply = httpClientGet(url,accessToken);
    if (reply == nullptr) 
    {
        qDebug() << "getdishwasherSignal->get : failed";
        return -1;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(statusCode == 401) 
    {
        if(retryTimes >= maxRetryTimes) 
        {
            qDebug() << "getdishwasherSignal->get : failed" << retryTimes << " times, but failed";
            return -1;
        }
        retryTimes++;
        if(getAccessToken() < 0)
        {
            qWarning()<< "getAccessToken failed";
            return -1;
        }        
        return getdishwasherSignal(pid); //重新尝试获取设备信息
    }
    else if (statusCode != 200) 
    {
        qWarning() << "getdishwasherSignal->get: failed,status code =" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return -1;
    }   

    qDebug() << "getdishwasherSignal->get succeed,statusCode = " << statusCode;
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
                    int tmpSignalValue = 0;
                    QJsonObject infoObj = obj.value("data").toObject();
                    if (infoObj.contains("signal"))
                    {
                        tmpSignalValue = infoObj.value("signal").toDouble();
                        qDebug() << "getdishwasherSignal->get succeed,tmpSignalValue = " << tmpSignalValue;
                        return tmpSignalValue;
                    }
                }
            }
        }
    }
    return -1;

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

    QString url = QString(HTTP_BASE_URL) + QString(GET_REGISTER_INFO);

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
    qDebug()<<"sendSelfCheckStatusToMes: pid = "<< pid;
    if(accessToken.isEmpty()) 
    {
        if(getAccessToken() < 0) 
        {
            qWarning()<< "getAccessToken failed";
            return false;
        }
    }

    QJsonObject json;
    json.insert("status",1);
    QJsonDocument doc;
    doc.setObject(json);
    QByteArray byteData = doc.toJson(QJsonDocument::Compact);

    QString url = QString(HTTP_BASE_URL) + QString(HTTP_PATCH_DOMAIN_SUFFIX_1) + \
                  QString(pid) + QString(HTTP_PATCH_DOMAIN_SUFFIX_2);
    qDebug() <<"sendSelfCheckStatusToMes: url = " << url;
    QNetworkReply *reply = httpClientPatchData(url,accessToken,byteData);
    if (reply == nullptr) 
    {
        qDebug() << "sendSelfCheckStatusToMes->patch： failed";
        return -1;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(statusCode == 401) 
    {
        if (retryTimes >= maxRetryTimes) 
        {
            qDebug() << "sendSelfCheckStatusToMes->patch:Get access token failed" << retryTimes << " times, but failed";
            return false;
        }
        retryTimes++;
        if(getAccessToken() < 0) 
        {
            qWarning()<< "getAccessToken failed";
            return false;
        }
        return sendSelfCheckStatusToMes(pid); //重新尝试
    }
    else if (statusCode != 200) 
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

PdDevSelfCheckInfo * PdHttpApi::getTcpClientSelfCheckResult(QString pid)
{
    qDebug()<<"getTcpClientSelfCheckResult : pid = "<< pid;
    if(accessToken.isEmpty()) 
    {
        if(getAccessToken() < 0) 
        {
            qWarning()<< "getAccessToken failed";
            return nullptr;
        }
    }    

    QString url = QString(HTTP_BASE_URL) + QString(HTTP_GET_INSPECT_STATUS_DOMAIN_SUFFIX_1) + \
                  QString(pid) + QString(HTTP_GET_INSPECT_STATUS_DOMAIN_SUFFIX_2);

    qDebug() <<"getTcpClientSelfCheckResult : url = " << url;
    QNetworkReply *reply = httpClientGet(url,accessToken);;
    if (reply == nullptr) 
    {
        qDebug() << "getTcpClientSelfCheckResult->get : failed";
        return nullptr;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode == 401) 
    {
        if (retryTimes >= maxRetryTimes) 
        {
            qDebug() << "getTcpClientSelfCheckResult->get:Get access token failed" << retryTimes << " times, but failed";
            return nullptr;
        }
        retryTimes++;        
        if(getAccessToken() < 0) 
        {
            qWarning()<< "getAccessToken failed";
            return nullptr;
        }        
        return getTcpClientSelfCheckResult(pid); //重新尝试
    }
    else if (statusCode != 200) 
    {
        qWarning() << "getTcpClientSelfCheckResult->get:failed,status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qWarning() << reply->readAll();
        return nullptr;
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
                    PdDevSelfCheckInfo * info =  new PdDevSelfCheckInfo;
                    QJsonObject infoObj = obj.value("data").toObject();
                    if (infoObj.contains("status")) 
                    {
                        info->selfCheckState = infoObj.value("status").toInt();
                    }
                    if (infoObj.contains("firmware_version")) 
                    {
                        info->dishwasherVersion = infoObj.value("firmware_version").toInt();
                    }
                    return info;                    
                }
            }
        }
    }

    return nullptr;    

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

