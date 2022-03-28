#include <QApplication>
#include "pdlog.h"
#include "pdhttpapi.h"
#include "mainwindow.h"
//#define DEBUG

//PdDeviceRegisterInfo AliRegisterInfoSt;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //DtuConfigTool w;
    MainWindow m;

    PdHttpApi::getInstance(&a);
//    PdHttpApi::getInstance(&a)->getTcpRegisterInfo("");

#ifdef DEBUG
    PdLog::getInstance()->init(PdLog::PdRelease, "PdLogTest.log")->install();

    //注意：这里仅仅是测试用
    PdDeviceRegisterInfo *info = PdHttpApi::getInstance(&a)->getRegisterInfo("690419270165589");
    if (info != nullptr) {
        PdHttpApi::getInstance(&a)->active("690419270165589");
        PdHttpApi::getInstance(&a)->getSelfCheckResult("690419270165589");
    }

#else

    PdLog::getInstance()->init(PdLog::PdDebug, "PdLogTest.log")->install();
#endif

    //w.show();
    m.setWindowTitle("DtuConfigTool");
    m.show();

    return a.exec();
}
