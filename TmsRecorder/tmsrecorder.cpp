#include "tmsrecorder.h"
#include <QFileDialog>
#include "../src/api/ITms2Player.h"
#include <iostream>
#include <QProcess>
#include <QMessageBox>
using namespace std;

void sleep(unsigned milliseconds);

TmsRecorder::TmsRecorder(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint );
	setWindowTitle(QString::fromLocal8Bit("转换录制文件"));
	setWindowIcon(QIcon(":/TmsRecorder/icon.ico"));
	ui.lineEdit_sourceFile->setReadOnly(true);
	connect(ui.btn_selSource, SIGNAL(clicked()), this, SLOT(slot_chooseSouceFile()));	
	connect(ui.btn_convert, SIGNAL(clicked()), this, SLOT(slot_start_convert()));
	
	m_trans = new CTrans(this);
	m_trans->moveToThread(&m_workThread);
	bool b = connect(&m_workThread, &QThread::finished, m_trans, &QObject::deleteLater);
	m_workThread.start();
	connect(m_trans, SIGNAL(sig_progress_value(int)), this, SLOT(slot_set_convert_progress(int)));
	connect(m_trans, SIGNAL(sig_error_occured(int)), this, SLOT(slot_error_occured(int)));
	connect(m_trans, SIGNAL(sig_convert_finished()), this, SLOT(slot_convert_finished()));
}

TmsRecorder::~TmsRecorder()
{
	m_workThread.quit();
	m_workThread.wait();
}

QString TmsRecorder::getSourceFile()
{
	return ui.lineEdit_sourceFile->text();
}

void TmsRecorder::setSourceFile(QString strFile)
{
	ui.lineEdit_sourceFile->setText(strFile);
	m_sourceFile = strFile;
}

void TmsRecorder::setParams(TmsParams params)
{
	//--cmd=t2m --tms=data/aaa.tms --mp4=data/aaaaaa
	m_sourceFile = params["--tms"];
	m_outputFile = params["--mp4"];
	m_bOnlyshowProgress = (params["--progress-only"].toLower() == "true")?true:false;
	setSourceFile(m_sourceFile);	
}

void TmsRecorder::showTmsRecorder()
{
	if (m_bOnlyshowProgress)
	{
		ui.stackedWidget->setCurrentIndex(1);
		slot_start_convert();
	}
	else
	{
		ui.stackedWidget->setCurrentIndex(0);
	}
	show();
}

void TmsRecorder::setOutPutPath()
{
	if (m_outputFile.isEmpty())
	{
		m_outputFile = QFileInfo(getSourceFile()).absolutePath();
	}
	QDir outDir(m_outputFile);
	outDir.mkdir("./");
	m_outputFile = outDir.absolutePath();
	if (!m_outputFile.isEmpty())
	{
		QFileInfo fInfo(m_outputFile);
		fInfo = QFileInfo(fInfo.absoluteFilePath());
		if (fInfo.isDir())
		{
			m_trans->setOutPutPath(m_outputFile);
		}
		else
		{
			m_trans->setOutPutPath(".");
			m_outputFile = ".";
		}
	}
	else
	{
		m_outputFile = ".";
	}
}

void TmsRecorder::slot_convert_finished()
{	
	//打开转换后文件夹
	QString src = getSourceFile();
	QFileInfo fInfo(m_outputFile);
	if (!fInfo.exists())
	{
		checkMode();
		return;
	}
	QProcess process;
	QString path = fInfo.absoluteFilePath();
#ifdef WIN32
	path.replace("/", "\\");    //***这句windows下必要***
#endif
	process.startDetached("explorer /," + path);

	checkMode();
}

void TmsRecorder::checkMode()
{
	if (m_bOnlyshowProgress)
	{
		QCoreApplication::quit();
	}
	else
	{
		ui.stackedWidget->setCurrentIndex(0);
	}
}

void TmsRecorder::slot_chooseSouceFile()
{
	QString path = ui.lineEdit_sourceFile->text();
	if (!QDir(path).exists())
	{
		path = ".";
	}
	QFileDialog *fd = new QFileDialog(this, QString::fromLocal8Bit("选择源文件"), path, "*.tms");
	fd->setFileMode(QFileDialog::ExistingFile);
	fd->setViewMode(QFileDialog::Detail);
	if (fd->exec())
	{
		QDir dir = fd->directory();
		QString absolutePath = dir.absolutePath();
		QStringList files = fd->selectedFiles();
		ui.lineEdit_sourceFile->setText(files.at(0));
	}
}

//void TmsRecorder::slot_setOutputFile()
//{
//	QString path = ui.lineEdit_outputFile->text();
//	if (!QDir(path).exists())
//	{
//		path = ".";
//	}
//	QString s = QFileDialog::getSaveFileName(this, tr("Save File"),
//		"untitled.mp4",
//		tr("MP4 (*.mp4)"));
//	QFileDialog *fd = new QFileDialog(this, QString::fromLocal8Bit("选择录屏文件目录"), path, "");
//	fd->setFileMode(QFileDialog::AnyFile);
//	fd->setViewMode(QFileDialog::Detail);
//	if (fd->exec())
//	{
//		QDir dir = fd->directory();
//		QString absolutePath = dir.absolutePath();
//		ui.lineEdit_outputFile->setText(absolutePath);
//	}
//}

void TmsRecorder::slot_start_convert()
{
	setOutPutPath();	
	emit sig_start_trans(getSourceFile());
}

void TmsRecorder::slot_set_convert_progress(int value)
{
	ui.stackedWidget->setCurrentIndex(1);
	ui.progressBar_3->setValue(value);
	ui.label_percent->setText(QString("%1%").arg(QString::number(value)));
}


void TmsRecorder::slot_error_occured(int errorCode)
{
	if (errorCode == FILE_NOT_FOUND)
	{				
		QMessageBox::warning(this, "error", QString::fromLocal8Bit("指定文件不存在，请检查后重试"));
	}
	else if (errorCode == FILE_NAME_ERROR)
	{
		QMessageBox::warning(this, "error", QString::fromLocal8Bit("文件名不能包含中文字符"));
	}
	checkMode();
}

/////////////////////////
CTrans::CTrans(TmsRecorder* recorder) :m_reoder(recorder)
{
	bool b = connect(m_reoder, SIGNAL(sig_start_trans(QString)), this, SLOT(transRecordFile(QString)));
}

CTrans::~CTrans()
{

}

void CTrans::setOutPutPath(QString outPath)
{
	m_outPutPath = outPath;
}

void CTrans::transRecordFile(QString sourceFile)
{
	QFileInfo fInfo(sourceFile);
	if (!fInfo.exists())
	{	
		emit sig_error_occured(FILE_NOT_FOUND);
		return;
	}
	ITms2Player* transfer = ITms2Player::create(sourceFile.toStdString(), false);
	if (transfer) {
		QString sOutPath = m_outPutPath.isEmpty() ? fInfo.absolutePath() : m_outPutPath;
		QString strOutFile = QString("%1/%2").arg(sOutPath, fInfo.completeBaseName());
		transfer->setOutputFile(strOutFile.toStdString());
		transfer->setVideoCrop(false);
		transfer->setVideoBgImage("./default_user2.png");
		transfer->setViewsBgImage("./default_user.png");
		//
		auto cb = [this](int percent) {
			emit sig_progress_value(percent % 100);
		};
		transfer->transfer(cb, 70);
	}
	else {
		emit sig_error_occured(FILE_NAME_ERROR);
		ITms2Player::destroy(transfer);
		return;	
	}
	while (transfer && transfer->isTransferring()) {
		sleep(30);
	}
	sleep(30);	
	ITms2Player::destroy(transfer);
	emit sig_convert_finished();
}


#ifdef RT_WIN32
#include <windows.h>
void sleep(unsigned milliseconds)
{
	Sleep(milliseconds);
}
#else
#include <unistd.h>

void sleep(unsigned milliseconds)
{
	usleep(milliseconds * 1000); // takes microseconds
}
#endif