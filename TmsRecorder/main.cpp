#include "tmsrecorder.h"
#include <QtWidgets/QApplication>
#include "./include/gflags/gflags.h"
#include <iostream>
using namespace std;

int  main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	//parse args
	QStringList arguments = QCoreApplication::arguments();
	QMap<QString, QString> argmap;
	for (int i = 1; i < arguments.count(); i++)
	{
		QString sarg = arguments.at(i);
		if (sarg.split("=").count() == 2)
		{
			QString skey = sarg.split("=").at(0);
			QString sValue = sarg.split("=").at(1);			
			argmap[skey] = sValue;
		}
	}

	TmsRecorder w;
	w.setParams(argmap);
	w.showTmsRecorder();
	return a.exec();
}

#if 0
const string g_empty_string("");

DEFINE_string(cmd, g_empty_string, "command");
DEFINE_string(tms, g_empty_string, "tms file name");
DEFINE_string(mp4, g_empty_string, "mp4 file name");
DEFINE_string(cdir, g_empty_string, "cache dir name, for restore");
DEFINE_int32(fcpu, 0, "free cpu level, default 0 (0--100)");

void sleep(unsigned milliseconds);
int  main(int argc, char *argv[]) {
	gflags::SetVersionString("1.0.0");
	gflags::SetUsageMessage("Usage: ./TmsRecorder");
	gflags::ParseCommandLineFlags(&argc, &argv, true);

	cout << endl;
	if (FLAGS_cmd == "dump" && !FLAGS_tms.empty()) {
		//ITmsFile::dumpTmsFile(cout, FLAGS_tms);
		return 0;
	}

	if (FLAGS_cmd == "restore" && !FLAGS_cdir.empty()) {
		//ITmsFile::restoreFromCache(cout, FLAGS_cdir);
		return 0;
	}
	QStringList arguments = QCoreApplication::arguments();

	//if (FLAGS_cmd == "t2m" && !FLAGS_tms.empty() && !FLAGS_tms.empty()) {
	{	ITms2Player* transfer = ITms2Player::create("./aaa.tms", false);
		if (transfer) {
			transfer->setOutputFile("./bcsedfd.mp4");
			transfer->setVideoCrop(false);
			//transfer->setVideoBgImage("data/video_bg.png");
			//transfer->setViewsBgImage("data/viewa_bg.png");
			//
			auto cb = [](int percent) {
				cout << " " << percent;
			};
			transfer->transfer(cb, 80);
			cout << "begin transfer... " << endl;
		}
		else {
			cout << "end" << endl;
		}
		while (transfer && transfer->isTransferring()) {
			sleep(30);
		}
		sleep(30);
		cout << "end" << endl;
		ITms2Player::destroy(transfer);
		return 0;
	}

	{
		cout << "error command." << endl;
	//	gflags::ShowUsageWithFlagsRestrict(argv[0], "TmsRecorder");
	}
	return 0;
}


#endif