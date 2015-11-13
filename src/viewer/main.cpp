#include "dialogs/MainWindow.h"

#include "FileLogListener.h"

#include <QApplication>

#include <sstream>

int main(int argc, char *argv[])
{
#ifdef Q_WS_X11
	bool useGUI = getenv("DISPLAY") != 0;
	if (!useGUI)
	{
		fprintf(stderr, "Fatal: You need a graphical window manager to work with this program.\n");
		return 1;
	}
#endif

	// Init logging system
	PR::FileLogListener fileLog;

	time_t t = time(NULL);
	std::stringstream sstream;
#ifdef PR_DEBUG
	sstream << "prv_" << t << "_d.log";
#else
	sstream << "prv_" << t << ".log";
#endif
	fileLog.open(sstream.str());
	PR_LOGGER.addListener(&fileLog);

	QApplication a(argc, argv);

	a.setApplicationName("PearRay Viewer");
	a.setApplicationVersion(PR_VERSION_STRING);
	a.setOrganizationName(PR_VENDOR_STRING);

	MainWindow w;
	w.show();
	return a.exec();
}
