#ifndef LOG_H
#define LOG_H

#include <QString>
#include <qlogging.h>


/**
 * @brief 使用时只需要在系统初始化最开始的位置install一下即可
 * 例如在main.cpp调用：
 * PdLog::getInstance()->init(PdLog::PdRelease, "PdLogTest.log")->install();
 */
class PdLog {
public:
    enum LogType {
        PdDebug,
        PdRelease
    };

private:
    PdLog(){}
    ~PdLog(){}

private:
    static PdLog *pdLogInstance;
    QString logPath = "PdLog.log";
    enum LogType logType = PdDebug;

public:
    /**
     * @brief getInstance 非线程安全，需要在程序初始化最开始的位置调用
     * @return
     */
    static PdLog *getInstance() {
        if (PdLog::pdLogInstance == nullptr) {
            PdLog::pdLogInstance = new PdLog();
        }
        return PdLog::pdLogInstance;
    }

    /**
     * @brief init 设置参数
     * @param type LogType为PdRelease时，log会保存到path文件中；LogType为PdDebug时，log会输出到控制台
     * @param path LogType为PdRelease时才会起作用
     * @return
     */
    PdLog *init(enum LogType type, const QString &path){
        logType = type;
        logPath = path;
        return pdLogInstance;
    }

    QString getLogPath() {
        return logPath;
    }

    /**
     * @brief install 根据参数设置log输出方式
     */
    void install();

};

#endif // LOG_H
