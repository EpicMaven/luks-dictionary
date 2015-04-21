#include "trydictionary.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QTextStream>
#include <iostream>
#include <QDebug>
#include <QTimer>

void usage() {
    std::cout << "luks-dictionary <password file> <fail file> <device/header>" << std::endl;
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("luks-dictionary");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("LUKS Dictionary password tester");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("passwordFilename", QCoreApplication::translate("main", "Password filename."));
    parser.addPositionalArgument("failFilename", QCoreApplication::translate("main", "Fail filename."));
    parser.addPositionalArgument("device", QCoreApplication::translate("main", "Device/header"));

    // A boolean option with a single name (-p)
    QCommandLineOption showProgressOption(QStringList() << "p" << "progress", QCoreApplication::translate("main", "Show progress"));
    parser.addOption(showProgressOption);

    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList args = parser.positionalArguments();
    // source is args.at(0), destination is args.at(1)

//    bool showProgress = parser.isSet(showProgressOption);

    QString passwordFilename = args.at(0);
    QString failFilename = args.at(1);
    QString device = args.at(2);

    if (passwordFilename == NULL || passwordFilename.size() <= 0) {
        std::cout << "Must specify password file" << std::endl;
        std::cout << "luks-dictionary <password file> <fail file> <device/header>" << std::endl;
        app.quit();
    }

    TryDictionary* tryDictionary = new TryDictionary(&app);
    tryDictionary->setPasswordFilename(passwordFilename);
    tryDictionary->setFailFilename(failFilename);
    tryDictionary->setDevice(device);

    QObject::connect(tryDictionary, SIGNAL(finished()), &app, SLOT(quit()));
    QTimer::singleShot(0, tryDictionary, SLOT(run()));

    return app.exec();
}
