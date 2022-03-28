#ifndef HTTPAPI_H
#define HTTPAPI_H
#include <QApplication>
#include "httpclient.h"
#include "deviceinfo.h"
#include <qDebug>
#include <QByteArray>
#include <QThread>
#define DTU_CFG_BY_TCP


#ifdef DTU_CFG_BY_TCP

#define DEV
//GET   https://dishwasher-oapi-dev.pudutech.com/dishwasher-gateway/api/v1/devices/69042110018001/dtu_config
//PATCH https://dishwasher-oapi-dev.pudutech.com/dishwasher-gateway/api/devices/69042110018001/activated_status
#ifdef DEV
//     #define HTTP_BASE_URL    "https://dishwasher-oapi-dev.pudutech.com"   // 开发服地址
        #define HTTP_BASE_URL   "https://dishwasher-oapi-test.pudutech.com"
//    #define HTTP_BASE_URL    "http://192.168.51.58:9001"
//    #define HTTP_BASE_URL     "http://localhost:8000"
//      #define HTTP_BASE_URL      "http://106.55.79.138:30012"
#elif defined TEST
    #define HTTP_BASE_URL    "https://dishwasher-oapi-test.pudutech.com"  // 测试服地址
#else
    #define HTTP_BASE_URL    "https://dishwasher-oapi.pudutech.com"       // 正式服地址
#endif

#define HTTP_GET_DTUCFG_DOMAIN_SUFFIX_1                    "/dishwasher-gateway/api/v1/devices/"
#define HTTP_GET_DTUCFG_DOMAIN_SUFFIX_2                    "/dtu_config"
#define HTTP_GET_INSPECT_STATUS_DOMAIN_SUFFIX_1            "/dishwasher-gateway/api/v1/devices/"
#define HTTP_GET_INSPECT_STATUS_DOMAIN_SUFFIX_2            "/inspect_status"          //"/inspect_status"
#define HTTP_PATCH_DOMAIN_SUFFIX_1                         "/dishwasher-gateway/api/v1/devices/"
#define HTTP_PATCH_DOMAIN_SUFFIX_2                         "/inspect_status"     //"/activated_status"

// #elif defined DTU_CFG_BY_HTTP

#ifdef DEV
#define BASE_URL        "https://cloud-oapi-dev.pudutech.com"
#define CLIENT_ID       "J8zm8uZP5SsEHLX"
#define CLIENT_SECRET   "a20eab00aa04e6a36cd991e1ad3fe086a384d64c"
#define GRANT_TYPE      "client_credentials"

//#define BASE_URL        "http://192.168.57.234:9000" //192.168.57.234 127.0.0.1

#elif defined TEST
#define BASE_URL        "https://cloud-oapi-test.pudutech.com"
#define CLIENT_ID       "J8zm8uZP5SsEHLX"
#define CLIENT_SECRET   "a20eab00aa04e6a36cd991e1ad3fe086a384d64c"
#define GRANT_TYPE      "client_credentials"
#else
#define BASE_URL        "https://cloud-oapi.pudutech.com"
#define CLIENT_ID       "tfALjNd6IJAZoam"
#define CLIENT_SECRET   "a485b43a3a50ceb68f79fffd31fa8fff22436da4"
#define GRANT_TYPE      "client_credentials"
#endif

#define AUTH_URL            "/oauth2/token"
#define GET_REGISTER_INFO   "/dishwasher-scheduler/api/v1/register_info"
#define ACTIVE_URL          "/dishwasher-scheduler/api/v1/activate"
#define SELF_CHECK_RESULT   "/dishwasher-scheduler/api/v1/get_activate_info"

#endif
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
     * @brief getRegisterInfo 获取三元组
     * @param pid
     * @return 返回三元组信息，返回值使用之后delete；获取失败则返回nullptr
     */
    PdDeviceRegisterInfo *getRegisterInfo(QString pid);

    /**
     * @brief getTcpRegisterInfo 获取TCP地址和秘钥
     * @param pid
     * @return 返回三元组信息，返回值使用之后delete；获取失败则返回nullptr
     */
    PdDeviceTcpRsgisterInfo *getTcpRegisterInfo(QString pid);

    /**
     * @brief active 激活
     * @param pid
     * @return true：激活成功；false：激活失败
     */
    bool active(QString pid);
    
    /**
     * @brief sendSelfCheckStatusToMes 通知MES,自检成功并入库
     * @param pid
     * @return true：入库成功；false：入库失败
     */    
    bool sendSelfCheckStatusToMes(QString pid);
    
    /**
     * @brief getSelfCheckResult 获取激活状态
     * @param pid
     * @return 0：未激活；1：已激活；-1：异常错误
     */
    int getSelfCheckResult(QString pid);

    /**
     * @brief getTcpClientSelfCheckResult 获取激活状态
     * @param pid
     * @return 0：未激活；1：已激活；-1：异常错误
     */    
    int getTcpClientSelfCheckResult(QString pid);

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
