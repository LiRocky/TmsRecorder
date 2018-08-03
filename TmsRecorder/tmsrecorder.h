#ifndef TMSRECORDER_H
#define TMSRECORDER_H
#include "transPage.h"

enum TRANS_ERROR
{
	FILE_NOT_FOUND = 1,
	FILE_NAME_ERROR = 2,
	TRANS_FAILED = 3,
};

//����ת���ļ�����

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
	QString m_outPutPath;//����ļ�·��
	bool m_timeStamp;
};

#endif // TMSRECORDER_H
