#ifndef TMSRECORDER_H
#define TMSRECORDER_H
#include "transPage.h"

enum TRANS_ERROR
{
	FILE_NOT_FOUND = 1,
	FILE_NAME_ERROR = 2,
	TRANS_FAILED = 3,
};

//用于转换文件的类

class CTrans :public QObject
{
	Q_OBJECT
public:
	CTrans(TmsRecorder*);
	~CTrans();
	void deleteLater();
	void setOutPutPath(QString);
	public slots:
	void transRecordFile(QString);
	void setTimestamp(bool);
signals:
	void sig_progress_value(int);
	void sig_error_occured(int);
	void sig_convert_finished();
private:
	TmsRecorder* m_reoder;
	QString m_outPutPath;//输出文件路径
	bool m_timeStamp;
};

#endif // TMSRECORDER_H
