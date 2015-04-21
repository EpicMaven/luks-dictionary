#include "trydictionary.h"

#include <QFile>
#include <QDebug>
#include <iostream>
#include <errno.h>

#include <QDateTime>
#include <QElapsedTimer>
#include <libcryptsetup.h>

TryDictionary::TryDictionary(QObject *parent) : QObject(parent)
{
}

bool TryDictionary::tryPassword(const QString& password) {
    bool wasSuccess = false;

    std::string passString = password.toStdString();
    const char* passwordPtr = passString.c_str();

    std::string deviceString = this->device.toStdString();
    const char* devicePtr = deviceString.c_str();

    struct crypt_device *cryptDevice;
    /* Decrypt the LUKS volume with the password */
    crypt_init(&cryptDevice, devicePtr);
    crypt_load(cryptDevice, CRYPT_LUKS1, NULL);
    // Only check passphrase (device_name == NULL)
    int ret = crypt_activate_by_passphrase(cryptDevice, NULL, CRYPT_ANY_SLOT, passwordPtr, password.size(), CRYPT_ACTIVATE_READONLY);

    /* Note: If the password works but the LUKS volume is already mounted,
       the crypt_activate_by_passphrase function should return -EBUSY. */
    if((ret >= 0) || (ret == -EBUSY))
    {
        wasSuccess = true;
        // only checking passphrase
        //crypt_deactivate(cryptDevice, device_name);
    }
    crypt_free(cryptDevice);

    return wasSuccess;
}

void TryDictionary::setPasswordFilename(const QString& passwordFilename) {
    this->passwordFilename = passwordFilename;
}

void TryDictionary::setFailFilename(const QString& failFilename) {
    this->failFilename = failFilename;
}

void TryDictionary::setDevice(const QString& device) {
    this->device = device;
}

qint64 TryDictionary::passwordCount(const QString& passwordFilename) {
    QFile passwordFile(passwordFilename);
    if (!passwordFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return -1;

    qint64 count = 0;
    QTextStream in(&passwordFile);
    while( !in.atEnd())
    {
        in.readLine();
        count++;
    }
    passwordFile.close();
    return count;
}

void TryDictionary::run() {
    std::cout << "passwordFile: " << this->passwordFilename.toStdString() << std::endl;
    std::cout << "failFilename: " << this->failFilename.toStdString() << std::endl;

    qint64 total = passwordCount(this->passwordFilename);
    if (total < 0)
        emit finished();

    QElapsedTimer totalTimer;
    QElapsedTimer timer;
    qint64 count = 1;

    QFile passwordFile(this->passwordFilename);
    if (!passwordFile.open(QIODevice::ReadOnly | QIODevice::Text))
        emit finished();
    QTextStream in(&passwordFile);

    QFile failFile(this->failFilename);
    if (!failFile.open(QIODevice::WriteOnly | QFile::Append | QIODevice::Text))
        emit finished();
    QTextStream failOut(&failFile);

    totalTimer.start();
    QString password = in.readLine();
    while (! password.isNull()) {
        timer.start();
        bool wasSuccess = tryPassword(password);
        qint64 elapsedMs = timer.elapsed();
        qint64 totalElapsedMs = totalTimer.elapsed();

        if (wasSuccess) {
            std::cout << "Success password: " << password.toStdString() << std::endl;

            QString successFilename = "success_password.txt";
            QFile successFile(successFilename);
            if (!successFile.open(QIODevice::WriteOnly | QFile::Append | QIODevice::Text))
                emit finished();
            QTextStream successOut(&successFile);
            successOut << password << endl;
            successOut << QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss.zzz") << endl;
            successFile.close();
            break;
        } else {
            QDateTime now = QDateTime::currentDateTime();
            qint64 avg = (totalElapsedMs / count);

            double percent = ((total - count) / (total * 1.0));

            qint64 etcMs = ((total - count) * avg);
            QDateTime etc = now.addMSecs(etcMs);

            std::cout << "failed: " << password.toStdString() << " | "
                      << count << " of " << total << " | "
                      << percent << "% | "
                      << elapsedMs << " ms | "
                      << avg << " ms/check | "
                      << etc.toString("dd-MM-yyyy hh:mm:ss.zzz").toStdString()
                      << std::endl;
            failOut << password << endl;
        }

        password = in.readLine();
        count++;
    }
    passwordFile.close();
    failFile.close();

    std::cout << "done." << std::endl;

    emit finished();
}

TryDictionary::~TryDictionary()
{

}

