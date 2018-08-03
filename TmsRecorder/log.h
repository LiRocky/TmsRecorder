#include <QFile>

bool InitLogFile();
void closeLogFile();
void writeLog(const QString& strMsg);

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);