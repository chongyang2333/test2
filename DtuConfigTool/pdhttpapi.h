#ifndef HTTPAPI_H
#define HTTPAPI_H
#include <QApplication>
#include "httpclient.h"
#include "deviceinfo.h"
#include <qDebug>
#include <QByteArray>
#include <QThread>

#define DEV
// #define TEST

#ifdef DEV
#define HTTP_BASE_URL   "https://dishwasher-oapi.pudutech.com"
#define HTTP_CLIENT_ID       "dw_client_prod_1649734583"
#define HTTP_CLIENT_SECRET   "evj5x2LcFb4e4VUe6w9C"

#elif defined TEST
#define HTTP_BASE_URL   "https://dishwasher-oapi-test.pudutech.com"
#define HTTP_CLIENT_ID       "dw_client_1648630378"
#define HTTP_CLIENT_SECRET   "5TQgVTNiEKp1PwMKaY5X"

#else
#define HTTP_BASE_URL   "https://dishwasher-oapi-test.pudutech.com"
#define HTTP_CLIENT_ID       "dw_client_1648630378"
#define HTTP_CLIENT_SECRET   "5TQgVTNiEKp1PwMKaY5X"
#endif

#define HTTP_GET_DTUCFG_DOMAIN_SUFFIX_1                    "/dishwasher-gateway/api/v1/devices/"
#define HTTP_GET_DTUCFG_DOMAIN_SUFFIX_2                    "/dtu_config"
#define HTTP_GET_INSPECT_STATUS_DOMAIN_SUFFIX_1            "/dishwasher-gateway/api/v1/devices/"
#define HTTP_GET_INSPECT_STATUS_DOMAIN_SUFFIX_2            "/inspect_status"          
#define HTTP_PATCH_DOMAIN_SUFFIX_1                         "/dishwasher-gateway/api/v1/devices/"
#define HTTP_PATCH_DOMAIN_SUFFIX_2                         "/inspect_status"     
#define HTTP_GET_DISHWASHER_SIGNAL_DOMAIN_SUFFIX_1         "/dishwasher-gateway/api/v1/devices/"
#define HTTP_GET_DISHWASHER_SIGNAL_DOMAIN_SUFFIX_2         "/signal_strength"
#define HTTP_POST_TOKEN_DOMAIN_SUFFIX                      "/dishwasher-gateway/api/v1/tokens"

/**
 * @brief 提供了当前需要使用的接口，所有接口都是同步的
 */
class PdHttpApi : public PdHttpClient {
public:
    /**
     * 注意：这个单例不是线程安全的
     */
    static PdHttpApi *getInstance(QObject *parent){
         if(pdHttpApi == nullptr) {
             pdHttpApi = new PdHttpApi(parent);
            //  qDebug()<<"creat PdHttpApi pid = "<<QThread::currentThread();
         }
        return pdHttpApi;
    }

    /**
     * @brief setMaxRetryTimes 如果查询到accessToken无效，最大的重试次数
     * @param times
     */
    void setMaxRetryTimes(int times){maxRetryTimes = times;}

    /**
     * @brief getTcpRegisterInfo 获取TCP地址和秘钥
     * @param pid
     * @return 返回三元组信息，返回值使用之后delete；获取失败则返回nullptr
     */
    PdDeviceTcpRsgisterInfo *getTcpRegisterInfo(QString pid);
    
    /**
     * @brief sendSelfCheckStatusToMes 通知MES,自检成功并入库
     * @param pid
     * @return true：入库成功；false：入库失败
     */    
    bool sendSelfCheckStatusToMes(QString pid);
    
    /**
     * @brief getTcpClientSelfCheckResult 获取激活状态
     * @param pid
     * @return 0：未激活；1：已激活；-1：异常错误
     */    
    PdDevSelfCheckInfo * getTcpClientSelfCheckResult(QString pid);

    /**
     * @brief getdishwasherSignal 获取整机信号强度
     * @param pid
     * @return >= 0：信号强度；-1：异常错误
     */
    int getdishwasherSignal(QString pid);

private:
    PdHttpApi(QObject *parent);
    PdHttpApi(const PdHttpApi&);
    ~PdHttpApi(){}
    int getAccessToken();

private:
    static PdHttpApi *pdHttpApi;
    QString accessToken;
    QString tokenType;
    qint32 expiresIn;
    int retryTimes;
    int maxRetryTimes;
};



#endif // HTTPAPI_H
