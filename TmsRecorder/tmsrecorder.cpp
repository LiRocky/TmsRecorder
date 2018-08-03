#include "tmsrecorder.h"
#include <QFileDialog>
#include "api\ITms2Player.h"
#include <iostream>
#include <QProcess>
#include <QMessageBox>
#include <tlhelp32.h>
#include <QtDebug> 
using namespace std;

void sleep(unsigned milliseconds);



CTrans::CTrans(TmsRecorder* recorder) :m_reoder(recorder)
{
	bool b = connect(m_reoder, SIGNAL(sig_start_trans(QString)), this, SLOT(transRecordFile(QString)), Qt::QueuedConnection);
}

CTrans::~CTrans()
{
	m_timeStamp = false;
}

void CTrans::deleteLater()
{
	QObject::deleteLater();
}

void CTrans::setOutPutPath(QString outPath)
{
	m_outPutPath = outPath;
}
#include <iostream>
#include <future>
void CTrans::transRecordFile(QString sourceFile)
{
	std::thread th([this,sourceFile]{
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
		
		transfer->setVideoCrop(false);
		transfer->setTimestamp(m_timeStamp);
		transfer->setMixSoleVideo(true);
		transfer->setScaleScreenv(true);
		//transfer->setVideoBgImage("./default_user.png");
		//transfer->setViewsBgImage("./default_user.png");
	
		auto cb = [this](void* userdata, int percent) {
			emit sig_progress_value(percent % 100);
		};
		if (!transfer->transfer(cb, 100))
		{
			emit sig_error_occured(TRANS_FAILED);
			sleep(30);
			ITmsTransfer::destroy(transfer);
		}		
	}
	else {
		emit sig_error_occured(TRANS_FAILED);
		sleep(30);
		ITmsTransfer::destroy(transfer);
		return;	
	}
	while (transfer && transfer->isTransferring()) {
		sleep(30);
	}
	//qDebug() << "trans finished";
		
//	qDebug() << "start trans destroy";
	transfer->cancel();
	ITmsTransfer::destroy(transfer);
	//qDebug() << "trans destroyed";
	emit sig_convert_finished();
	sleep(30);
	});
	th.detach();
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


