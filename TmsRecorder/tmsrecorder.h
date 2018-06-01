#ifndef TMSRECORDER_H
#define TMSRECORDER_H
#include <QThread>
#include <QtWidgets/QMainWindow>
#include "ui_tmsrecorder.h"
#include <QMap>
#include "../src/api/ITms2Player.h"




enum TRANS_ERROR
{
	FILE_NOT_FOUND = 1,
	FILE_NAME_ERROR = 2,
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
signals:
	void sig_progress_value(int);
	void sig_error_occured(int);
	void sig_convert_finished();
private:
	TmsRecorder* m_reoder;
	QString m_outPutPath;//����ļ�·��
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
	void slot_start_convert();
	void slot_set_convert_progress(int);
	void slot_error_occured(int);
	void slot_convert_finished();
private:
	void checkMode();//�жϵ�ǰģʽ
signals:
	
	void sig_start_trans(QString);
private:
	QString m_sourceFile;
	QString m_outputFile;
	CTrans* m_trans;
	QThread m_workThread;
	bool m_bOnlyshowProgress;//	ֻ����ʾת�����ȣ����ṩѡ���ļ�����
private:
	Ui::TmsRecorderClass ui;
};



#endif // TMSRECORDER_H
