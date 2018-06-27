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
	//ui.lineEdit_sourceFile->setReadOnly(true);
	connect(ui.btn_selSource, SIGNAL(clicked()), this, SLOT(slot_chooseSouceFile()));	
	connect(ui.btn_convert, SIGNAL(clicked()), this, SLOT(slot_convert_singleFile()));
	
	m_trans = new CTrans(this);
	m_trans->moveToThread(&m_workThread);
	bool b = connect(&m_workThread, &QThread::finished, m_trans, &QObject::deleteLater);
	m_workThread.start();
	b=connect(m_trans, SIGNAL(sig_progress_value(int)), this, SLOT(slot_set_convert_progress(int)));
	b=connect(m_trans, SIGNAL(sig_error_occured(int)), this, SLOT(slot_error_occured(int)));
	connect(m_trans, SIGNAL(sig_convert_finished()), this, SLOT(slot_convert_finished()));
}

TmsRecorder::~TmsRecorder()
{
	m_workThread.quit();
	m_workThread.wait();
}

QString TmsRecorder::getSourceFile()
{
	if (m_sourceList.size() > 0)
	{
		return m_sourceList.first();
	}
	else
	{
		return QString("");
	}
}

void TmsRecorder::setSourceFile(QString strFile)
{
	ui.lineEdit_sourceFile->setText(strFile);
	m_sourceFile = strFile;
}

void TmsRecorder::setParams(TmsParams params)
{
	//--cmd=t2m --tms=data/aaa.tms --mp4=data/aaaaaa
	m_multiSouceFiles = params["--multiTms"];
	m_sourceFile = params["--tms"];
	m_outputFile = params["--mp4"];
	m_bOnlyshowProgress = (params["--progress-only"].toLower() == "true") ? true : false;
	m_sourceFolder = params["--tmsFolder"];
	m_addTimeStamp = (params["--addTimeStamp"].toLower() == "true") ? true : false;
	QDir sourceDir(m_sourceFolder);
	if (sourceDir.exists())
	{
		QFileInfoList newFileList = sourceDir.entryInfoList();
		int childCount = sourceDir.entryInfoList().count();
		for (int i = 0; i < childCount; i++)
		{
			if (newFileList.at(i).suffix().operator ==("tms"))
			{
				QString abPath = newFileList.at(i).absoluteFilePath();
				m_sourceList.append(abPath);
			}						
		}
		//return;
	}
	if (false == m_sourceFile.isEmpty())
	{
		m_sourceList.append(m_sourceFile);
	}
	if (false == m_multiSouceFiles.isEmpty())
	{
		QStringList tmsFiles = m_multiSouceFiles.split("&");
		m_sourceList.append(tmsFiles);
	}
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
		if (m_outputFile.contains(QRegExp("[\\x4e00-\\x9fa5]+")))
		{
			slot_error_occured(FILE_NAME_ERROR);
			return;
		}
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
#include <QTextStream>
void TmsRecorder::slot_convert_finished()
{
	QFileInfo fio = QFileInfo(getSourceFile());
	QFile file(fio.absolutePath()+"/trans.txt");
	if (file.open(QIODevice::ReadWrite | QIODevice::Append))
	{
		QTextStream out(&file);		
		out << fio.fileName();
		out << "\n";
		file.close();
	}
	QDir srcDir = QFileInfo(getSourceFile()).absoluteDir();
	//srcDir.remove(getSourceFile());
	m_sourceList.removeFirst();
	m_sucCount++;
	QString stip;
	stip = QString("%1/%2").arg(m_sucCount).arg(m_totalCount);
	ui.label_sucTip->setText(stip);
	if (m_totalCount > (m_sucCount + m_errorCount))
	{
		emit sig_start_trans(getSourceFile());
	}
	else if (m_totalCount == (m_sucCount + m_errorCount))
	{
		//打开转换后文件夹
		openFolder(m_outputFile);
		checkMode();
		
	}

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
		m_outputFile.clear();
	}
}

void TmsRecorder::slot_chooseSouceFile()
{
	QString path = ui.lineEdit_sourceFile->text();
	if (!QDir(path).exists())
	{
		path = ".";
	}
	QStringList files = QFileDialog::getOpenFileNames(
		this,
		"Select one or more files to open",
		"./",
		"Source File(*.tms)");
	m_sourceList = files;
	
		if (m_sourceList.size()<=1)
		{
			ui.lineEdit_sourceFile->setText(getSourceFile());
		}
		else
		{
			slot_start_convert();
		}		
}

void TmsRecorder::slot_convert_singleFile()
{
	m_sourceList.clear();
	QString srcFile = ui.lineEdit_sourceFile->text();
	QFileInfo fio(srcFile);
	if (fio.isFile())
	{
		if (fio.exists() )
		{
			if ( fio.completeSuffix().contains("tms"))
			{
				m_sourceList.append(srcFile);
				slot_start_convert();
			}
			else
			{
				QMessageBox::warning(this, "error", QString::fromLocal8Bit("不是有效的源文件，请检查后重试"));
			}			
		}
		else
		{
			QMessageBox::warning(this, "error", QString::fromLocal8Bit("指定文件不存在，请检查后重试"));
		}
	}
	else
	{
		QMessageBox::warning(this, "error", QString::fromLocal8Bit("不是有效的源文件，请检查后重试"));
	}
}

void TmsRecorder::slot_start_convert()
{
	if (m_sourceList.size() == 0)
	{
		return;
	}
	m_sucCount = 0;
	m_errorCount = 0;
	m_totalCount = m_sourceList.size();
	ui.stackedWidget->setCurrentIndex(1);
	QString stip;
	stip = QString("%1/%2").arg(m_sucCount).arg(m_totalCount);
	ui.label_sucTip->setText(stip);
	ui.progressBar_3->setValue(0);
	setOutPutPath();	
	m_trans->setTimestamp(m_addTimeStamp);
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
		QMessageBox::warning(this, "error", QString::fromLocal8Bit("文件名或路径不能包含中文字符"));
	}
	else if (errorCode == TRANS_FAILED)
	{
		QMessageBox::warning(this, "error", QString::fromLocal8Bit("转码失败"));
	}
	m_sourceList.removeFirst();
	m_errorCount++;
	if (m_totalCount > (m_sucCount + m_errorCount))
	{
		emit sig_start_trans(getSourceFile());
	}
	else if (m_totalCount == (m_sucCount + m_errorCount))
	{
		
		//打开转换后文件夹	
		if (m_sucCount>0)
		{
			openFolder(m_outputFile);
		}	
		checkMode();
	}	
}

void TmsRecorder::openFolder(QString strFolder)
{
	QFileInfo fInfo(strFolder);
	if (fInfo.exists())
	{
		QProcess process;
		QString path = fInfo.absoluteFilePath();
#ifdef WIN32
		path.replace("/", "\\");
#endif
		process.startDetached("explorer /," + path);
		return;
	}
}
#include <QCloseEvent>
void TmsRecorder::closeEvent(QCloseEvent *event)
{
	if (ui.stackedWidget->currentIndex() == 1)
	{
		int ret = QMessageBox::warning(this, "warning", QString::fromLocal8Bit("是否终止此次转换"), QMessageBox::Ok, QMessageBox::Cancel);
		if (ret == QMessageBox::Ok)
		{
			event->accept();
			QCoreApplication::quit();
		}
		else
		{
			event->ignore();
			show();
			activateWindow();
		}
	}
	
}

/////////////////////////
CTrans::CTrans(TmsRecorder* recorder) :m_reoder(recorder)
{
	bool b = connect(m_reoder, SIGNAL(sig_start_trans(QString)), this, SLOT(transRecordFile(QString)));
}

CTrans::~CTrans()
{
	m_timeStamp = false;
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
	if (fInfo.absoluteFilePath().contains(QRegExp("[\\x4e00-\\x9fa5]+")))
	{
		emit sig_error_occured(FILE_NAME_ERROR);
		return;
	}
	ITms2Player* transfer = ITms2Player::create(sourceFile.toStdString(), false);
	if (transfer) {
		QString sOutPath = m_outPutPath.isEmpty() ? fInfo.absolutePath() : m_outPutPath;
		QString strOutFile = QString("%1/%2").arg(sOutPath, fInfo.completeBaseName());
		transfer->setOutputFile(strOutFile.toStdString());
		transfer->setVideoCrop(true);
		transfer->setTimestamp(m_timeStamp);
		//transfer->setVideoBgImage("./default_user.png");
		//transfer->setViewsBgImage("./default_user.png");
	
		auto cb = [this](int percent) {
			emit sig_progress_value(percent % 100);
		};
		transfer->transfer(cb, 80);
	}
	else {
		emit sig_error_occured(TRANS_FAILED);
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


void CTrans::setTimestamp(bool timestamp)
{
	m_timeStamp = timestamp;
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