#include "trydictionary.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include <iostream>
#include <QDebug>
#include <QTimer>
#include <QFileInfo>

bool fileExists(const QString& filename) {
    bool exists = false;
    QFileInfo checkFile(filename);

    if (checkFile.exists() && checkFile.isFile()) {
        exists = true;
    }

    return exists;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("luks-dictionary");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("LUKS dictionary based password tester");
    parser.addPositionalArgument("passwordFilename", QCoreApplication::translate("main", "Password dictionary filename."));
    parser.addPositionalArgument("failFilename", QCoreApplication::translate("main", "Fail log filename."));
    parser.addPositionalArgument("device", QCoreApplication::translate("main", "LUKS device or header (e.g., /dev/sda1, backup-headers)"));

    QCommandLineOption progressOption(QStringList() << "p" << "progress", QCoreApplication::translate("main", "show progress. Default: true"));
    parser.addOption(progressOption);

    QCommandLineOption noProgressOption(QStringList() << "P" << "no-progress", QCoreApplication::translate("main", "hide progress."));
    parser.addOption(noProgressOption);

    parser.addHelpOption();
    parser.addVersionOption();


    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList args = parser.positionalArguments();

    bool showProgress = true;
    if (parser.isSet(progressOption)) {
        showProgress = true;
    } else if (parser.isSet(noProgressOption)) {
        showProgress = false;
    }

    if (args.size() < 1) {
        std::cout << "Must specify password dictionary file" << std::endl;
        parser.showHelp();
        app.quit();
    }
    QString passwordFilename = args.at(0);
    if (! fileExists(passwordFilename)) {
        std::cout << "Password dictionary file does not exist" << std::endl;
        parser.showHelp();
        app.quit();
    }

    if (args.size() < 2) {
        std::cout << "Must specify fail log file" << std::endl;
        parser.showHelp();
        app.quit();
    }
    QString failFilename = args.at(1);

    if (args.size() < 3) {
        std::cout << "Must specify LUKS device or header file" << std::endl;
        parser.showHelp();
        app.quit();
    }
    QString device = args.at(2);
    if (! fileExists(device)) {
        std::cout << "LUKS device or header file does not exist" << std::endl;
        parser.showHelp();
        app.quit();
    }

    TryDictionary* tryDictionary = new TryDictionary(&app);
    tryDictionary->setPasswordFilename(passwordFilename);
    tryDictionary->setFailFilename(failFilename);
    tryDictionary->setDevice(device);
    tryDictionary->setProgress(showProgress);

    QObject::connect(tryDictionary, SIGNAL(finished()), &app, SLOT(quit()));
    QTimer::singleShot(0, tryDictionary, SLOT(run()));

    return app.exec();
}
