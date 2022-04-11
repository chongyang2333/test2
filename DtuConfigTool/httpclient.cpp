#include "httpclient.h"
#include <QBuffer>
#include <QDebug>
#include <QUrl>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
PdHttpClient::PdHttpClient(QObject *parent, bool sync) : QObject(parent)
{
    // qDebug()<<"PdHttpClient pid = "<<QThread::currentThread();
    manager = new QNetworkAccessManager(parent);    
//    manager->moveToThread(parent);
    syncFlag = sync;
    qDebug() << manager->supportedSchemes();
    // qDebug() << "start creat manager,syncflag = "<<syncFlag;

    if (syncFlag) {
        /**
         * @brief 槽函数，在异步调用时使用
         */
            qDebug() << "start connect manger slot";
        connect(manager, &QNetworkAccessManager::finished, this, &PdHttpClient::receiveReply);
    }
}

PdHttpClient::~PdHttpClient()
{

}

QNetworkReply *PdHttpClient::httpClientGetData(QString url)
{
    QNetworkRequest request;
    
    // request.setUrl(QUrl("http://localhost:8000/dishwasher-gateway/api/v1/devices/69042110018001/dtu_config"));
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    // request.setRawHeader("pid", pid.toLocal8Bit());

    if (syncFlag) 
    {
        // qDebug() << "httpClientGetData sync";
        return manager->get(request);
    } else 
    {
        // qDebug() << "httpClientGetData Nosync";
        QNetworkReply *reply = manager->get(request);
        return waitHttpReply(reply);
    }
}

QNetworkReply *PdHttpClient::httpClientGet(QString url, QString auth)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("bearer " + auth).toLocal8Bit());

    if (syncFlag) {
        return manager->get(request);
    } else {
        QNetworkReply *reply = manager->get(request);
        return waitHttpReply(reply);
    }
}

QNetworkReply *PdHttpClient::httpClientPatchData(QString url,QString auth,QByteArray data)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("bearer " + auth).toLocal8Bit());
    
    QByteArray * bufArray = new QByteArray();
    *bufArray = data;
    QBuffer *tmpBuf = new QBuffer( bufArray);;

    // qDebug()<<"buf size = "<<tmpBuf->size()<<data<<"data size"<<data.size();
    if (syncFlag) 
    {
        return manager->sendCustomRequest(request,QByteArray("PATCH"),tmpBuf);
    }
    else 
    {
        QNetworkReply *reply = manager->sendCustomRequest(request,QByteArray("PATCH"),tmpBuf);
        return waitHttpReply(reply);
    }
}

QNetworkReply *PdHttpClient::httpClientPost(QString url, QString auth, QByteArray data)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if(auth != nullptr)
    {
        request.setRawHeader("Authorization", ("bearer " + auth).toLocal8Bit());
    }

    if(syncFlag)
    {
        return manager->post(request, data);
    }
    else
    {
        QNetworkReply *reply = manager->post(request, data);
        return waitHttpReply(reply);
    }
}

QNetworkReply *PdHttpClient::receiveReply(QNetworkReply *reply)
{
    qDebug()<<"operation:" << reply->operation();
    qDebug()<<"status code: " << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

//    const QByteArray replyData = reply->readAll();
//    qDebug()<<"reply data: " << replyData;

    reply->deleteLater();
    return reply;
}

QNetworkReply *PdHttpClient::waitHttpReply(QNetworkReply *reply)
{
    QTimer timer;
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::finished, &eventLoop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &eventLoop, &QEventLoop::quit);
    timer.setSingleShot(true);
    timer.start(HTTP_TIMEOUT_MS);
    eventLoop.exec();
    if (timer.isActive()) {
        timer.stop();
        return receiveReply(reply);
    }else{
            const QByteArray replyData = reply->readAll();
            qDebug()<<"reply data: " << replyData;
        qDebug() << "Http post timeout";
        return nullptr;
    }
}
