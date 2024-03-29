#include "dialogs/MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QSplashScreen>
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
	parser.addPositionalArgument("materials", QApplication::translate("main", "Material name"), "[materials...]");

	parser.process(app);

	// OpenGL
	QSurfaceFormat format;
	format.setDepthBufferSize(24);
	format.setStencilBufferSize(8);
	format.setVersion(3, 2);
	format.setProfile(QSurfaceFormat::CoreProfile);
	QSurfaceFormat::setDefaultFormat(format);

	QPixmap pixmap(":/splash_materiallab");
	QSplashScreen splash(pixmap);
	splash.show();

	// Main Window
	MainWindow w;
	w.show();

	splash.repaint();
	splash.raise();
	app.processEvents();

	for (const QString& item : parser.positionalArguments())
		w.newInspection(item);

	w.raise();
	splash.finish(&w);
	
	return app.exec();
}