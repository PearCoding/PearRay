#include "dialogs/MainWindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QFileInfo>
#include <QSurfaceFormat>

// We do not link to the library, only include the configuration file!
#include "PR_Config.h"

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

	app.setApplicationName("PearRayDiagnostic");
	app.setApplicationVersion(PR_VERSION_STRING);
	app.setOrganizationName(PR_VENDOR_STRING);

	// Command Line
	QCommandLineParser parser;
	parser.setApplicationDescription("PearRay diagnose tool");
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("input", QApplication::translate("main", "Input file or directory"));

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

	if (!parser.positionalArguments().empty()) {
		QFileInfo info(parser.positionalArguments().first());
		if (info.exists()) {
			if (info.isFile()) {
				w.openFile(info.filePath());
			} else if (info.isDir()) {
				w.openRDMPDir(info.filePath());
			}
		}
	}
	return app.exec();
}