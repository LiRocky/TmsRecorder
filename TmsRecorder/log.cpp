#include "log.h"
#include <QDateTime>
#include <QTextStream>
#include <QMutex>
#include <QDir>

QFile* g_logFile = NULL;
QMutex mutex;
bool InitLogFile()
{

	QDir logDir(".");
	if (!logDir.exists())
	{
		logDir.mkpath("./");
	}
	
	QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmmss");
	QString logFile = "./" + strDateTime + ".txt";
	mutex.lock();
	g_logFile = new QFile(logFile);
	if (g_logFile == NULL)
	{
		return false;
	}
	if (!g_logFile->open(QIODevice::ReadWrite | QIODevice::Append))
	{
		return false;
	}
	mutex.unlock();
	return true;
}

void closeLogFile()
{

}

void writeLog(const QString& strMsg)
{
	if (g_logFile == NULL)
	{
		InitLogFile();
	}
	mutex.lock();
	if (g_logFile != NULL && g_logFile->isOpen())
	{
		QTextStream stream(g_logFile);
		stream << strMsg << "\r\n";
		g_logFile->flush();
	}
	mutex.unlock();
}

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QByteArray localMsg = msg.toLocal8Bit();
	QString strMsg("");
	switch (type) {
	case QtDebugMsg:
		strMsg = QString("Debug:");
		break;
	case QtWarningMsg:
		strMsg = QString("Warning:");
		break;
	case QtCriticalMsg:
		strMsg = QString("Critical:");
		break;
	case QtFatalMsg:
		strMsg = QString("Fatal:");
		break;
	}
	// 设置输出信息格式	
	QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
	QString strMessage = QString("[%5][%2][%4]==[%1]")
		.arg(localMsg.constData()).arg(context.file).arg(context.function).arg(strDateTime);
	writeLog(strMessage);
	return;
}
