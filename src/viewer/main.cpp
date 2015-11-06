#include "dialogs/MainWindow.h"

#include <QApplication>

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

	QApplication a(argc, argv);

	a.setApplicationName("PearRay Viewer");
	a.setApplicationVersion(PR_VERSION_STRING);
	a.setOrganizationName(PR_VENDOR_STRING);

	MainWindow w;
	w.show();
	return a.exec();
}
