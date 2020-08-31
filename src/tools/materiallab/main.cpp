#include "dialogs/MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QSurfaceFormat>

#include "config/Version.h"

int main(int argc, char** argv)
{
#ifdef Q_WS_X11
	bool useGUI = getenv("DISPLAY") != 0;
	if (!useGUI) {
		fprintf(stderr, "Fatal: You need a graphical window manager to work with this program.\n");
		return 1;
	}
#endif

	QApplication app(argc, argv);

	app.setApplicationName("PearRayMaterialLab");
	app.setApplicationVersion(PR_VERSION_STRING);
	app.setOrganizationName(PR_VENDOR_STRING);

	// Command Line
	QCommandLineParser parser;
	parser.setApplicationDescription("PearRay Material Lab");
	parser.addHelpOption();
	parser.addVersionOption();

	parser.process(app);

	// OpenGL
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);

	// Main Window
	MainWindow w;
	w.show();
	return app.exec();
}