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

void TryDictionary::setProgress(bool showProgress) {
    this->showProgress = showProgress;
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

void TryDictionary::displaySummary() {
    if (! this->showProgress)
        return;
    std::cout << "passwordFile   : " << this->passwordFilename.toStdString() << std::endl;
    std::cout << "failFilename   : " << this->failFilename.toStdString() << std::endl;
    std::cout << "total passwords: " << this->totalPasswords << std::endl;
}

void TryDictionary::displayFailProgress(const QString& password) {
    if (! this->showProgress)
        return;

    qint64 totalElapsedMs = this->totalTimer.elapsed();
    qint64 avg = (totalElapsedMs / this->triedCount);

    double percent = (this->triedCount/(this->totalPasswords * 1.0)) * 100.0;
    QString percentStr;
    percentStr.sprintf("%.6f", percent);
    percentStr = percentStr.rightJustified(10, ' ');

    QString triedStr;
    triedStr.setNum(this->triedCount);
    triedStr = triedStr.rightJustified(this->totalPasswordsStrLength, ' ');

    qint64 etcMs = ((this->totalPasswords - this->triedCount) * avg);
    QDateTime now = QDateTime::currentDateTime();
    QDateTime etc = now.addMSecs(etcMs);

    std::cout << "failed: " << password.toStdString() << " | "
              << triedStr.toStdString() << " of " << this->totalPasswords << " | "
              << percentStr.toStdString() << "% | "
              << this->elapsedTimer.elapsed() << " ms | "
              << avg << " ms/check | "
              << etc.toString("dd-MM-yyyy hh:mm:ss.zzz").toStdString()
              << std::endl;
}

void TryDictionary::displaySuccessProgress(const QString& password) {
    if (! this->showProgress)
        return;
    std::cout << "Success password: " << std::endl
              << password.toStdString() << std::endl;
}

void TryDictionary::displayDone() {
    if (! this->showProgress)
        return;

    std::cout << std::endl
              << "Total elapsed " << this->totalTimer.elapsed() << " ms" << std::endl
              << "Done." << std::endl;
}

void TryDictionary::run() {
    this->totalPasswords = passwordCount(this->passwordFilename);
    if (this->totalPasswords <= 0)
        emit finished();

    QString totalStr;
    totalStr.setNum(this->totalPasswords);
    this->totalPasswordsStrLength = totalStr.size();

    this->triedCount = 0;

    displaySummary();

    QFile passwordFile(this->passwordFilename);
    if (!passwordFile.open(QIODevice::ReadOnly | QIODevice::Text))
        emit finished();
    QTextStream in(&passwordFile);

    QFile failFile(this->failFilename);
    if (!failFile.open(QIODevice::WriteOnly | QFile::Append | QIODevice::Text))
        emit finished();
    QTextStream failOut(&failFile);

    this->totalTimer.start();
    QString password = in.readLine();
    while (! password.isNull()) {
        elapsedTimer.start();
        bool wasSuccess = tryPassword(password);
        this->triedCount++;

        if (wasSuccess) {
            displaySuccessProgress(password);

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
            displayFailProgress(password);
            failOut << password << endl;
        }

        password = in.readLine();
    }
    passwordFile.close();
    failFile.close();

    displayDone();
    emit finished();
}

TryDictionary::~TryDictionary()
{

}
