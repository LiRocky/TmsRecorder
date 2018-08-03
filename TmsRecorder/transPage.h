#ifndef _TRANS_PAGE_H
#define  _TRANS_PAGE_H
#include <QThread>
#include <QtWidgets/QMainWindow>
#include "ui_tmsrecorder.h"
#include <QMap>
#include "../src/api/ITms2Player.h"
#include <QStringList>

//转码文件选择和转码提示界面
//显示界面
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
	void checkMode();//判断当前模式
	void openFolder(QString);
	void closeEvent(QCloseEvent *event);
signals:

	void sig_start_trans(QString);
private:
	//QString m_sourceFile;
	QString m_multiSouceFiles;	//包含多个tms文件字符串，利用&分隔
	QString m_outputFile;		//输出目录
	CTrans* m_trans;			//实际完成转码的类
	QThread m_workThread;		//转码工作线程
	bool m_bOnlyshowProgress;	//只是显示转换进度，不提供选择文件功能
	bool m_addTimeStamp;		//添加时间戳选项
	QStringList m_sourceList;	//
	//QString m_sourceFolder;
	int m_totalCount;			//参与转码的tms文件总数
	int m_sucCount;				//转码成功的个数
	int m_errorCount;			//转码失败的个数

private:
	Ui::TmsRecorderClass ui;
};

#endif // !_TRANS_PAGE_H
