


#include "pdlog.h"

#include <QDebug>
#include <QMutex>
#include <QString>
#include <QMessageLogContext>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>

#include <stdio.h>

PdLog *PdLog::pdLogInstance = nullptr;

/**
 * @brief formatLog对log进行格式化，比如我这里给log添加了文件名和行数、时间
 * @param type log严重类型
 * @param context log输出上下文，包含文件路径、行数、函数名等，目前看在release编译后，获取不到
 * @param msg log数据
 * @return 格式化后的数据
 */
static QString formatLog(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString text;

    switch (type) {
    case QtDebugMsg:
        text = QString("Debug:");
        break;
    case QtWarningMsg:
        text = QString("Warning:");
        break;
    case QtCriticalMsg:
        text = QString("Critical:");
        break;
    case QtFatalMsg:
        text = QString("Fatal:");
        break;
    default:
        text = QString("  ");
        break;
    }
    QString fileName = QFileInfo(context.file).fileName();
    QString contextInfo = QString("%1:%2").arg(fileName).arg(context.line);

    QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    QString message = QString("%1 %2 %3 %4").arg(time).arg(text).arg(contextInfo).arg(msg);

    return message;
}

/**
 * @brief outputToFile 保存log到文件，参数与上面的formatLog相同
 * @param type
 * @param context
 * @param msg
 */
void outputToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;

    mutex.lock();

    QString message = formatLog(type, context, msg);

    QString path = PdLog::getInstance()->getLogPath();
    QFile file(path);
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream textStream(&file);
    textStream << message << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();
}

/**
 * @brief outputToStd 输出log到标准输出，参数与上面的formatLog相同
 * @param type
 * @param context
 * @param msg
 */
void outputToStd(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;

    mutex.lock();

    QString message = formatLog(type, context, msg);

    printf("%s\n", message.toStdString().data());

    fflush(stdout);

    mutex.unlock();
}


void PdLog::install()
{
    if (logType == PdLog::PdRelease) {
        qInstallMessageHandler(outputToFile);
    } else {
        qInstallMessageHandler(outputToStd);
    }
}

