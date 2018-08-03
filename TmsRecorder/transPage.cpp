#include "transPage.h"

#include "tmsrecorder.h"
#include <QFileDialog>
#include <iostream>
#include <QProcess>
#include <QMessageBox>
#include <tlhelp32.h>
#include <QtDebug> 
using namespace std;


int killTaskl(const QString& exe);

//////////////////////////////////////////////////////////////////////////
/////////////////////////转换文件选择和提示界面/////////////////////////////
//////////////////////////////////////////////////////////////////////////

TmsRecorder::TmsRecorder(QWidget *parent)
: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowFlags(windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
	setWindowTitle(QString::fromLocal8Bit("转换录制文件"));
	setWindowIcon(QIcon(":/TmsRecorder/icon.ico"));
	//ui.lineEdit_sourceFile->setReadOnly(true);
	connect(ui.btn_selSource, SIGNAL(clicked()), this, SLOT(slot_chooseSouceFile()));
	connect(ui.btn_convert, SIGNAL(clicked()), this, SLOT(slot_convert_singleFile()));

	m_trans = new CTrans(this);
	//m_trans->moveToThread(&m_workThread);
	//bool b = connect(&m_workThread, &QThread::finished, m_trans, &QObject::deleteLater);
	//m_workThread.start();
	bool b = connect(m_trans, SIGNAL(sig_progress_value(int)), this, SLOT(slot_set_convert_progress(int)));
	b = connect(m_trans, SIGNAL(sig_error_occured(int)), this, SLOT(slot_error_occured(int)));
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

}

void TmsRecorder::setParams(TmsParams params)
{
	//--cmd=t2m --tms=data/aaa.tms --mp4=data/aaaaaa
	m_multiSouceFiles = params["--multiTms"];
	//m_outputFile = params["--mp4"];
	m_bOnlyshowProgress = (params["--progress-only"].toLower() == "true") ? true : false;
	m_addTimeStamp = (params["--addTimeStamp"].toLower() == "true") ? true : false;

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
	QFile file(fio.absolutePath() + "/trans.txt");
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

	if (m_sourceList.size() <= 1)
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
		if (fio.exists())
		{
			if (fio.completeSuffix().contains("tms"))
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
	ui.label_percent->setText("");
	setOutPutPath();
	m_trans->setTimestamp(m_addTimeStamp);
	if (!getSourceFile().isEmpty())
	{
		m_trans->transRecordFile(getSourceFile());
		Sleep(30);
	}	
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
	if (!m_sourceList.isEmpty())
	{
		m_sourceList.removeFirst();
	}
	
	m_errorCount++;
	if (m_totalCount > (m_sucCount + m_errorCount))
	{
		emit sig_start_trans(getSourceFile());
	}
	else if (m_totalCount == (m_sucCount + m_errorCount))
	{

		//打开转换后文件夹	
		if (m_sucCount > 0)
		{
			openFolder(m_outputFile);
		}
		checkMode();
	}
}

void TmsRecorder::openFolder(QString strFolder)
{
	if (!ui.checkBox_openFolder->isChecked())
	{
		return;
	}
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
			killTaskl("TmsRecorder.exe");
		}
		else
		{
			event->ignore();
			show();
			activateWindow();
		}
	}

}



int killTaskl(const QString& exe)
{
	//1、根据进程名称找到PID
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return -1;
	}

	BOOL    bRet = FALSE;
	DWORD dwPid = -1;
	while (Process32Next(hProcessSnap, &pe32))
	{
		//将WCHAR转成const char*
		int iLn = WideCharToMultiByte(CP_UTF8, 0, const_cast<LPWSTR> (pe32.szExeFile), static_cast<int>(sizeof(pe32.szExeFile)), NULL, 0, NULL, NULL);
		std::string result(iLn, 0);
		WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, static_cast<int>(sizeof(pe32.szExeFile)), const_cast<LPSTR> (result.c_str()), iLn, NULL, NULL);
		if (0 == strcmp(exe.toStdString().c_str(), result.c_str()))
		{
			dwPid = pe32.th32ProcessID;
			bRet = TRUE;
			//qDebug() << "zhaodao";
			break;
		}
	}

	CloseHandle(hProcessSnap);
	//qDebug() << dwPid;

	HANDLE hProcess = NULL;
	//打开目标进程
	hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, dwPid);
	if (hProcess == NULL) {
		//qDebug() << "Open Process fAiled ,error:" << GetLastError();
		return -1;
	}
	//结束目标进程
	DWORD ret = TerminateProcess(hProcess, 0);
	if (ret == 0) {
		//qDebug() << "kill task faild,error:" << GetLastError();
		return -1;
	}

	return 0;
}