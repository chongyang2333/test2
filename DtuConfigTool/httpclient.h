#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QObject>


#define HTTP_TIMEOUT_MS     (25*1000)

class PdHttpClient: public QObject
{
    Q_OBJECT
public:
    explicit PdHttpClient(QObject *parent, bool sync);
    ~PdHttpClient();
    QNetworkReply *httpClientGetData(QString url);
    QNetworkReply *httpClientPatchData(QString url,QString auth,QByteArray data);
    QNetworkReply *httpClientGet(QString url, QString auth);
    QNetworkReply *httpClientPost(QString url, QString auth, QByteArray data);


public slots:
    QNetworkReply *receiveReply(QNetworkReply *reply);

private:
    QNetworkReply *waitHttpReply(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QTimer timer;
    bool syncFlag;
};


#endif // HTTPCLIENT_H
