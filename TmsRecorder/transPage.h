#ifndef _TRANS_PAGE_H
#define  _TRANS_PAGE_H
#include <QThread>
#include <QtWidgets/QMainWindow>
#include "ui_tmsrecorder.h"
#include <QMap>
#include "../src/api/ITms2Player.h"
#include <QStringList>

//ת���ļ�ѡ���ת����ʾ����
//��ʾ����
class CTrans;
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
	//QString m_sourceFile;
	QString m_multiSouceFiles;	//�������tms�ļ��ַ���������&�ָ�
	QString m_outputFile;		//���Ŀ¼
	CTrans* m_trans;			//ʵ�����ת�����
	QThread m_workThread;		//ת�빤���߳�
	bool m_bOnlyshowProgress;	//ֻ����ʾת�����ȣ����ṩѡ���ļ�����
	bool m_addTimeStamp;		//���ʱ���ѡ��
	QStringList m_sourceList;	//
	//QString m_sourceFolder;
	int m_totalCount;			//����ת���tms�ļ�����
	int m_sucCount;				//ת��ɹ��ĸ���
	int m_errorCount;			//ת��ʧ�ܵĸ���

private:
	Ui::TmsRecorderClass ui;
};

#endif // !_TRANS_PAGE_H
