#include <QtCore>
#include <QLocale>
#include <QApplication>
#include <QSplashScreen>
#include <QDir>
#include <QBitmap>

#include "Fragmentarium/GUI/MainWindow.h"

// Needed for unicode commandline below.
#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    qApp->addLibraryPath("./");
    qApp->addLibraryPath("./plugins");
    qApp->addLibraryPath("iconengines");
    qApp->addLibraryPath("imageformats");
    qApp->addLibraryPath("platforms");
#endif


    Q_INIT_RESOURCE(Fragmentarium);


    QApplication::setStyle(QStyleFactory::create(QString("Fusion"))); // default gui style

    /// space in the name seemed to cause problems with reading and writing ~/.config/Syntopia Software/
    /// replaced with "_" fixed preferences settings not loading...
    /// was saved in "~/.config/Syntopia Software/" tried to load from "~/.config/Syntopia/"
    QApplication::setOrganizationName(QString("Syntopia_Software"));
    QApplication::setApplicationName(QString("Fragmentarium"));

    QApplication *app = new QApplication(argc, argv);
   
    app->setApplicationVersion("2.5.0.190304");

    // this should translate all of the generic default widget texts
    QTranslator qtTranslator;
    qtTranslator.load(QString("qt_") + QLocale::system().name(),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    app->installTranslator(&qtTranslator);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.setApplicationDescription(
        QString("\n") + app->translate("main", "Fragmentarium is a cross-platform IDE for exploring pixel based GPU graphics.")
    );

    parser.addPositionalArgument(QString("filename.frag"), app->translate("main", "initial fragment to open.") + QString("\n"), QString("[filename.frag]") );

//     parser.addOption(QCommandLineOption("nograb","tells Qt that it must never grab the mouse or the keyboard."));
//     parser.addOption(QCommandLineOption("dograb","(only under X11), running under a debugger can cause an implicit -nograb, use -dograb to override."));
//     parser.addOption(QCommandLineOption("sync","(only under X11), switches to synchronous mode for debugging."));
    parser.addOption(QCommandLineOption (QString("verbose"),
                                         app->translate("main", "sets reporting of shader variables to console."),
                                         QString(""),
                                         QString(""))
                    );

    parser.addOption(QCommandLineOption (QString("style"),
                                         app->translate("main", "sets the application GUI style.\nPossible values are '")+QStyleFactory::keys().join("','")+"'.",
                                         QString("style"),
                                         QString("Fusion"))
                    );
//     parser.addOption(QCommandLineOption("stylesheet","stylesheet, sets the application styleSheet. The value must be a path to a file that contains the Style Sheet."));
//     parser.addOption(QCommandLineOption("session","restores the application from an earlier session."));
//     parser.addOption(QCommandLineOption("widgetcount","prints debug message at the end about number of widgets left undestroyed and maximum number of widgets existed at the same time"));
//     parser.addOption(QCommandLineOption("reverse","sets the application's layout direction to Qt::RightToLeft"));
//     parser.addOption(QCommandLineOption("graphicssystem","sets the backend to be used for on-screen widgets and QPixmaps. Available options are raster and opengl."));
//     parser.addOption(QCommandLineOption("qmljsdebugger","activates the QML/JS debugger with a specified port. The value must be of format port:1234[,block], where block is optional and will make the application wait until a debugger connects to it."));

//     parser.addOption(QCommandLineOption("display","sets the X display (default is $DISPLAY)."));
//     parser.addOption(QCommandLineOption("geometry","sets the client geometry of the first window that is shown."));
//     parser.addOption(QCommandLineOption("font","defines the application font. The font should be specified using an X logical font description. Note that this option is ignored when Qt is built with fontconfig support enabled."));
//     parser.addOption(QCommandLineOption("background","sets the default background color and an application palette (light and dark shades are calculated)."));
//     parser.addOption(QCommandLineOption("foreground","sets the default foreground color."));
//     parser.addOption(QCommandLineOption("button"," sets the default button color."));
//     parser.addOption(QCommandLineOption("name","sets the application name."));
//     parser.addOption(QCommandLineOption("title","sets the application title."));
//     parser.addOption(QCommandLineOption("visual","TrueColor, forces the application to use a TrueColor visual on an 8-bit display."));
//     parser.addOption(QCommandLineOption("ncols","limits the number of colors allocated in the color cube on an 8-bit display, if the application is using the QApplication::ManyColor color specification. If count is 216 then a 6x6x6 color cube is used (i.e. 6 levels of red, 6 of green, and 6 of blue); for other values, a cube approximately proportional to a 2x3x1 cube is used."));
//     parser.addOption(QCommandLineOption("cmap","causes the application to install a private color map on an 8-bit display."));
//     parser.addOption(QCommandLineOption("im","sets the input method server (equivalent to setting the XMODIFIERS environment variable)"));
//     parser.addOption(QCommandLineOption("inputstyle","defines how the input is inserted into the given widget, e.g., onTheSpot makes the input appear directly in the widget, while overTheSpot makes the input appear in a box floating over the widget and is not inserted until the editing is done."));

    parser.addOption(QCommandLineOption( (QStringList() << QString("l") << QString("language")),
                                         app->translate("main", "sets the application language.\nPossible values are 'en','de','ru','nl'."),
                                         QString("language"),
                                         QString("en"))
                    );

    parser.addOption(QCommandLineOption( (QStringList() << QString("s") << QString("script")),
                                         app->translate("main", "Fragmentarium script file to load. Must be \".fqs\" filename extention."),
                                         QString("script"),
                                         QString(""))
                    );

    // Process the actual command line arguments given by the user
    parser.process( app->arguments() );

    QString langArg = QString("en");

    QTranslator myappTranslator;
    if(parser.isSet(QString("language"))) {
        langArg = parser.value(QString("language"));
        if( langArg != QString("en") ) {
            if(myappTranslator.load(QString("Languages/Fragmentarium_") + langArg ))
                app->installTranslator(&myappTranslator);
            else
                qDebug() << QString("Can't find Fragmentarium_%1.qm !!!").arg(langArg);
        }
    }
    else {
        langArg = QLocale::system().name().split("_").at(0);
        if( langArg != QString("en") ) {
            if(myappTranslator.load(QString("Languages/Fragmentarium_") + langArg))
                app->installTranslator(&myappTranslator);
            else
                qDebug() << QString("Can't find Fragmentarium_%1.qm !!!").arg(langArg);
        }
    }

    QPixmap pixmap(QDir(Fragmentarium::GUI::MainWindow::getMiscDir()).absoluteFilePath("splash.png"));
    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);

    Fragmentarium::GUI::MainWindow *mainWin;
    mainWin = new Fragmentarium::GUI::MainWindow(&splash);
    mainWin->setDockOptions(QMainWindow::AllowTabbedDocks|QMainWindow::AnimatedDocks);
    mainWin->langID = langArg;

    mainWin->setVerbose(parser.isSet("verbose"));

    mainWin->show();

    splash.setMask(pixmap.mask());
    QStringList openFiles = QSettings().value("openFiles").toStringList();

    if(!parser.isSet("script") || openFiles.isEmpty()) splash.show();

    QStringList args = parser.positionalArguments();
    QString fragFile = args.isEmpty() ? QString() : args.last();
    /// load a single frag from comandline or load the default bulb
    if( !fragFile.isEmpty() ) {
        mainWin->loadFragFile( app->arguments().last() );
    }
    else if(openFiles.count() > 0) {

        splash.finish(mainWin);
        while(openFiles.count() > 0) {
            mainWin->loadFragFile(openFiles.first());
            openFiles.removeFirst();
        }
    } else
        mainWin->loadFragFile(QDir(mainWin->getExamplesDir()).absoluteFilePath("Historical 3D Fractals/Mandelbulb.frag"));

    // needs here on windows or script control gets priority over gui refresh
    app->processEvents();
    app->flush();

    if(parser.isSet("script")) {
        QString filename = parser.value("script");
        if(filename.endsWith(".fqs")) {

            QFile file(filename);

            if(file.exists()) {
                if (file.open(QFile::ReadOnly | QFile::Text)) {
                    splash.finish(mainWin);
                    QTextStream in(&file);
                    QString text = in.readAll();
                    file.close();
                    // The sync function will first empty Qts events by calling QCoreApplication::processEvents(),
                    // then the platform plugin will sync up with the windowsystem,
                    // and finally Qts events will be delived by another call to QCoreApplication::processEvents();
                    app->sync();
                    // everything is now in place and ready for script control
                    mainWin->runScript( text );
                } else qDebug() << "Script file " << filename << " open failed!";
            } else qDebug() << "Script file " << filename << " does not exist!";
        } else qDebug() << "Script file requires .fqs extention!";
    } else mainWin->setSplashWidgetTimeout(&splash);

/// BEGIN 3DTexture
//     if(app.arguments().contains("-voxel")) {
//       int argIndex = app.arguments().indexOf("-voxel");
//       QString filename = app.arguments().at(argIndex+1);
//       if(filename.endsWith(".exr")) {
//           mainWin->setVoxelFile(filename);
//           qDebug() << "EXR Voxel file name set.";
//       } else qDebug() << "Wrong file type, should be an EXR file that contains sub-images.";
//     }
//
//     if(app.arguments().contains("-obj")) {
//       int argIndex = app.arguments().indexOf("-obj");
//       QString filename = app.arguments().at(argIndex+1);
//       if(filename.endsWith(".obj")) {
//         mainWin->setObjFile(filename);
//         qDebug() << "OBJ file name set.";
//       } else qDebug() << "Wrong file type, should be an OBJ file.";
//     }
/// END 3DTexture

    return app->exec();
}


