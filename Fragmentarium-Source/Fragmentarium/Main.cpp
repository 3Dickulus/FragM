#include <QApplication>
#include <QBitmap>
#include <QDir>
#include <QLocale>
#include <QSplashScreen>
#include <QtCore>

#include "GUI/MainWindow.h"

#ifdef Q_OS_WIN
// Needed for unicode commandline below.
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
// https://fractalforums.org/fragmentarium/17/example-selecting-nvidia-gpu-on-a-laptop-with-two-gpus/2694/msg13596#msg13596
// extern "C" {
//   _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
// }

#else

// segfault signal handler prints a nicer message
#include <csignal>
#include <unistd.h>
void segv_handler(int s)
{
    // can't use most functions in a signal handler, see `man signal-safety`
    const char *message =
        "Segmentation fault.\n"
        "Fragmentarium crashed!\n"
        "For advice, please visit:\n"
        "OpenGL Version: <https://en.wikibooks.org/wiki/Fractals/fragmentarium#Troubleshooting>\n"
        "and...\n"
        "GPU Watchdog: <http://blog.hvidtfeldts.net/index.php/2011/12/fragmentarium-faq/>\n"
        "\n"
        "If you have found an error, please report the following:\n"
        "– OS type and OS version\n"
        "– Graphics card type and driver version\n"
        "– The version of Fragmentarium, and whether you built it yourself\n"
        "– A reproducible description of the steps that caused the error (if possible).\n"
        "\n"
        "If you have an account at github you can post in <https://github.com/3Dickulus/FragM/issues>\n"
        "or file a bug report at <https://github.com/3Dickulus/FragM>\n"
        "You can also find discussions about Fragmentarium at <https://fractalforums.org/fragmentarium/17>\n"
        "\n";
    write(2 /* stderr FD */, message, strlen(message));
    abort();
}

#endif

enum CommandLineParseResult
{
        CommandLineOk,
        CommandLineError,
        CommandLineVersionRequested,
        CommandLineHelpRequested
};

bool last_run_state;

CommandLineParseResult parseCommandLine(QCommandLineParser &parser, QString *errorMessage)
{
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

///////////////////////////////////////  other qt options
//     parser.addOption(QCommandLineOption("nograb","tells Qt that it must never grab the mouse or the keyboard."));
//     parser.addOption(QCommandLineOption("dograb","(only under X11), running under a debugger can cause an implicit -nograb, use -dograb to override."));
//     parser.addOption(QCommandLineOption("sync","(only under X11), switches to synchronous mode for debugging."));
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

    parser.addPositionalArgument("filename.frag", qApp->translate("main", "initial fragment to open.") + QString("\n"), QString("[filename.frag]"));

    const QCommandLineOption verboseOption (QString("verbose"),
                                         qApp->translate("main", "Sets reporting of shader variables and other things to console."),
                                         QString("bool"),
                                         QString("true"));
    parser.addOption(verboseOption);

    const QCommandLineOption autorunOption (QStringList() << QString("a") << QString("autorun"),
                                         qApp->translate("main", "Execute auto-compile-run cycle at program start."),
                                         QString("bool"),
                                         QString("true"));
    parser.addOption(autorunOption);

    const QCommandLineOption styleOption (QString("style"),
                                         qApp->translate("main", "Sets the application GUI style.\nPossible values are '")+QStyleFactory::keys().join("','")+"'.",
                                         QString("stylename"),
                                         QString("Fusion"));
    parser.addOption(styleOption);

    const QCommandLineOption stylesheetOption ("qstylesheet",
                                         qApp->translate("main", "Sets the application stylesheet. The value must be a path to a valid .qss file."),
                                         QString("qss filename"),
                                         QString(""));
    parser.addOption(stylesheetOption);

    const QCommandLineOption editorthemeOption ("editortheme",
                                         qApp->translate("main", "Sets the colour theme for the text editor. Possible values are Default, Dark, Light, Retta"),
                                         QString("theme"),
                                         QString("Default"));
    parser.addOption(editorthemeOption);

    const QCommandLineOption languageOption ( (QStringList() << QString("l") << QString("language")),
                                         qApp->translate("main", "sets the application language.\nPossible values are 'en','de','ru','nl'."),
                                         QString("language"),
                                         QString("en"));
    parser.addOption(languageOption);

    const QCommandLineOption scriptOption ( (QStringList() << QString("s") << QString("script")),
                                         qApp->translate("main", "Fragmentarium script file to load. Must be \".fqs\" filename extention."),
                                         QString("fqs filename"),
                                         QString(""));
    parser.addOption(scriptOption);

    const QCommandLineOption compatpatchOption (QStringList() << QString("c") << QString("compatpatch"),
                                         qApp->translate("main", "Attempt to allow legacy shaders to run under modern GL core profile."),
                                         QString("bool"),
                                         QString("true"));
    parser.addOption(compatpatchOption);

    if (!parser.parse(QApplication::arguments())) {
        *errorMessage = parser.errorText();
        return CommandLineError;
    }

    if(parser.isSet(helpOption)) return CommandLineHelpRequested;
    if(parser.isSet(versionOption)) return CommandLineVersionRequested;

    if( parser.isSet(QString("autorun")) ) {
        printf( "%s", qPrintable(qApp->translate("main", "Automatically compile and run GLSL is ")) );
        if(parser.value(QString("autorun")) == "false") {
            QSettings().setValue("autorun", false );
            printf( "%s\n", qPrintable(qApp->translate("main", "disabled.")) );
        } else {
            QSettings().setValue("autorun", true );
            printf( "%s\n", qPrintable(qApp->translate("main", "enabled.")) );
        }
    }
    
    if( parser.isSet(QString("compatpatch")) ) {
        printf( "%s", qPrintable(qApp->translate("main", "Compatibility patch ")) );
        if(parser.value(QString("compatpatch")) == "false") {
            QSettings().setValue("compatPatch", false );
            printf( "%s\n", qPrintable(qApp->translate("main", "disabled.")) );
        } else {
            QSettings().setValue("compatPatch", true );
            printf( "%s\n", qPrintable(qApp->translate("main", "enabled.")) );
        }
    }

    if( parser.isSet(QString("qstylesheet")) ) {
        if(!parser.value(QString("qstylesheet")).isEmpty()) {
            QString ssfilename = parser.value(QString("qstylesheet"));
            if(!ssfilename.isEmpty()) {
                QFile f(ssfilename);
                if (!f.exists())
                {
                    printf("%s\n", qPrintable(qApp->translate("main", "Unable to set stylesheet ") + ssfilename + qApp->translate("main", " file not found!")));
                }
                else
                {
                    printf("%s\n", qPrintable(qApp->translate("main", "Stylesheet set to ") + ssfilename));
                    qApp->setStyleSheet("file:///"+ssfilename);
                    QSettings().setValue("guiStylesheet", ssfilename );
                }
            }
        } else {
            QSettings().setValue("isStarting", last_run_state);
            qApp->setStyleSheet("");
            printf("%s\n", qPrintable(qApp->translate("main", "Stylesheet unset.")));
        }
    }

    if( parser.isSet(QString("editortheme")) ) {
        int theme = 0;
        QString themeName = parser.value(QString("editortheme"));
        if(themeName==qApp->translate("main", "Default")) theme = 0;
        if(themeName==qApp->translate("main", "Dark")) theme = 1;
        if(themeName==qApp->translate("main", "Light")) theme = 2;
        if(themeName==qApp->translate("main", "Retta")) theme = 3;
        QSettings().setValue("editorTheme", theme );

        printf("%s\n", qPrintable(qApp->translate("main", "Frag editor theme set to ") + themeName));
    }
    
    const QStringList positionalArguments = parser.positionalArguments();
    if (positionalArguments.isEmpty()) {
        *errorMessage = qApp->translate("main", "Argument 'filename' missing.");
        return CommandLineOk;
    } else if (positionalArguments.size() > 1) {
        *errorMessage = qApp->translate("main", "Too many arguments specified.");
        return CommandLineError;
    }

    return CommandLineOk;
}


int main(int argc, char *argv[])
{

#ifdef Q_OS_WIN
    qApp->addLibraryPath("./");
    qApp->addLibraryPath("./plugins");
    qApp->addLibraryPath("iconengines");
    qApp->addLibraryPath("imageformats");
    qApp->addLibraryPath("platforms");
#else
    // install signal handler to catch segmentation faults if not windows
    signal(SIGSEGV, segv_handler);
#endif

#ifdef Q_OS_MAC
    qApp->addLibraryPath("../Frameworks");
    qApp->addLibraryPath("../PlugIns");
    qApp->addLibraryPath("../PlugIns/iconengines");
    qApp->addLibraryPath("../PlugIns/imageformats");
    qApp->addLibraryPath("../PlugIns/platforms");
    qApp->addLibraryPath("../PlugIns/printsupprort");
    qApp->addLibraryPath("../PlugIns/platforminputcontexts");
    qApp->addLibraryPath("../PlugIns/styles");
    qApp->addLibraryPath("../PlugIns/virtualkeyboard");
#endif

    Q_INIT_RESOURCE(Fragmentarium);

    qApp->setOrganizationName(QString("Syntopia_Software"));
    qApp->setApplicationName(QString("Fragmentarium"));
    qApp->setOrganizationDomain(QString("fractalforums.org"));

    QScopedPointer<QApplication> app(new QApplication(argc, argv));
    app->setObjectName("Application");

    // before creating main window record the last run state
    last_run_state = QSettings().value("isStarting").toBool();

    QCommandLineParser parser;
    parser.setApplicationDescription(QString("\n") + app->translate("main", "Fragmentarium is a cross-platform IDE for exploring pixel based GPU graphics."));

    // this checks the commandline options for proper format and flags, shows help text and error message then exits if anything is wrong
    QString errorMessage;
    switch(parseCommandLine(parser, &errorMessage)) {
       case CommandLineOk: break;
       case CommandLineError:
            QSettings().setValue("isStarting", last_run_state);
            fputs("\n\n", stderr);
            fputs(qPrintable(errorMessage), stderr);
            fputs("\n\n", stderr);
            fputs(qPrintable(parser.helpText()), stderr);
            return 1;
        case CommandLineVersionRequested:
            QSettings().setValue("isStarting", last_run_state);
            printf("%s %s\n", qPrintable(QCoreApplication::applicationName()),
                    qPrintable(QCoreApplication::applicationVersion()));
            return 0;
        case CommandLineHelpRequested:
            QSettings().setValue("isStarting", last_run_state);
            parser.showHelp();
            // exits in showHelp function and never returns here
            Q_UNREACHABLE();
    }
    
    QString langArg = QString("en"); // default english
    QTranslator myappTranslator;
    if(parser.isSet(QString("language"))) {
        if( !parser.value(QString("language")).isEmpty()) langArg = parser.value(QString("language"));
        if( langArg != QString("en") ) {
            if (myappTranslator.load(QString("Languages/Fragmentarium_") + langArg)) {
                app->installTranslator(&myappTranslator);
                printf("%s\n",qPrintable(app->translate("main", "Using ") + langArg + app->translate("main", " language file.")));
            } else {
                printf("%s\n",qPrintable(app->translate("main", "Language ") + langArg + app->translate("main", " failed!!!")));
            }
        }
    }

    QPixmap pixmap(QDir(Fragmentarium::GUI::MainWindow::getMiscDir()).absoluteFilePath("splash.png"));
    QSplashScreen splash(pixmap, Qt::WindowStaysOnTopHint);

    splash.setMask(pixmap.mask());
    splash.show();

    Fragmentarium::GUI::MainWindow *mainWin;
    mainWin = new Fragmentarium::GUI::MainWindow();

    // Makes the splash screen wait until the widget mainWin is displayed before calling close() on itself.
    splash.finish(mainWin);

    mainWin->setObjectName("MainWindow");

    app->setApplicationVersion(mainWin->getVersion());

    mainWin->setLanguage(langArg);

    mainWin->setDockOptions(QMainWindow::AllowTabbedDocks | QMainWindow::AnimatedDocks);

    mainWin->setVerbose(parser.isSet("verbose"));

    mainWin->setWindowTitle(QString("%1 %2").arg(qApp->applicationName()).arg(qApp->applicationVersion()));

    mainWin->show();

    mainWin->readSettings();
    
    QStringList openFiles = (parser.isSet("script")) ? QStringList() : QSettings().value("openFiles").toStringList();

    QStringList args = parser.positionalArguments();
    QString fragFile = args.isEmpty() ? QString() : args.last();
    /// load a single frag from comandline
    if( !fragFile.isEmpty() ) {
        mainWin->loadFragFile( fragFile );
    } else if (openFiles.count() > 0 && !parser.isSet("script")) {

        while(openFiles.count() > 0) {
            mainWin->loadFragFile(openFiles.first());
            openFiles.removeFirst();
        }
    } else { // load the default bulb
        mainWin->loadFragFile(QDir(mainWin->getExamplesDir()).absoluteFilePath("Historical 3D Fractals/Mandelbulb.frag"));
    }

    // needed here on windows or script control gets priority over gui refresh
    app->processEvents();

    if(parser.isSet("script")) {
        QString filename = parser.value("script");
        if(filename.endsWith(".fqs")) {

            QFile file(filename);

            if(file.exists()) {
                if (file.open(QFile::ReadOnly | QFile::Text)) {
                    QTextStream in(&file);
                    QString text = in.readAll();
                    file.close();
                    // The sync function will first empty Qts events by calling QCoreApplication::processEvents(),
                    // then the platform plugin will sync up with the windowsystem,
                    // and finally Qt events will be delivered by another call to QCoreApplication::processEvents();
                    app->sync();
                    // everything is now in place and ready for script control
                    mainWin->runScript( text );
                } else {
                    printf("%s",qPrintable(app->translate("main","Script file ") + filename + app->translate("main"," open failed!\n")));
                    exit(0);
                }
            } else {
                printf("%s",qPrintable(app->translate("main","Script file ") + filename + app->translate("main"," does not exist!\n")));
                exit(0);
            }
        } else {
            printf("%s",qPrintable(app->translate("main","Script file requires .fqs extention!\n")));
            exit(0);
        }
    } else {
        return app->exec();
    }
    exit(0);
}


