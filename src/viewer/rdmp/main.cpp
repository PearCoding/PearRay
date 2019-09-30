#include "dialogs/MainWindow.h"

#include <QApplication>

// We do not link to the library, only include the configuration file!
#include "PR_Config.h"

int main(int argc, char* argv[])
{
#ifdef Q_WS_X11
	bool useGUI = getenv("DISPLAY") != 0;
	if (!useGUI) {
		fprintf(stderr, "Fatal: You need a graphical window manager to work with this program.\n");
		return 1;
	}
#endif

	QApplication a(argc, argv);

	a.setApplicationName("PearRay RDMP Viewer");
	a.setApplicationVersion(PR_VERSION_STRING);
	a.setOrganizationName(PR_VENDOR_STRING);

	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);

	MainWindow w;
	w.show();

	if (argc == 2) {
		w.openProject(argv[1], true);
	}

	return a.exec();
}
