#ifndef TMSRECORDER_H
#define TMSRECORDER_H
#include <QThread>
#include <QtWidgets/QMainWindow>
#include "ui_tmsrecorder.h"
#include <QMap>
#include "../src/api/ITms2Player.h"
#include <QStringList>



enum TRANS_ERROR
{
	FILE_NOT_FOUND = 1,
	FILE_NAME_ERROR = 2,
	TRANS_FAILED = 3,
};

//����ת���ļ�����
class TmsRecorder;
class CTrans :public QObject
{
	Q_OBJECT
public:
	CTrans(TmsRecorder*);
	~CTrans();
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


//��ʾ����
class TmsRecorder : public QMainWindow
{
	Q_OBJECT
	typedef QMap<QString, QString> TmsParams;
public:
	TmsRecorder(QWidget *parent = 0);
	~TmsRecorder();
public:
	QString getSourceFile();
	void setSourceFile(QString strFile);
	
	void setParams(TmsParams);
	void showTmsRecorder();
	void setOutPutPath();
	protected slots:
	void slot_chooseSouceFile();
	//void slot_setOutputFile();
	void slot_convert_singleFile();
	void slot_start_convert();
	void slot_set_convert_progress(int);
	void slot_error_occured(int);
	void slot_convert_finished();
private:
	void checkMode();//�жϵ�ǰģʽ
	void openFolder(QString);
	void closeEvent(QCloseEvent *event);
signals:
	
	void sig_start_trans(QString);
private:
	QString m_sourceFile;
	QString m_multiSouceFiles;
	QString m_outputFile;
	CTrans* m_trans;
	QThread m_workThread;
	bool m_bOnlyshowProgress;//	ֻ����ʾת�����ȣ����ṩѡ���ļ�����
	bool m_addTimeStamp;
	QStringList m_sourceList;
	QString m_sourceFolder;
	int m_totalCount;
	int m_sucCount;
	int m_errorCount;

private:
	Ui::TmsRecorderClass ui;
};



#endif // TMSRECORDER_H
