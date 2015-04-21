#ifndef TRYDICTIONARY_H
#define TRYDICTIONARY_H

#include <QElapsedTimer>
#include <QObject>

class TryDictionary : public QObject
{
    Q_OBJECT
public:
    explicit TryDictionary(QObject *parent = 0);
    ~TryDictionary();

    void setPasswordFilename(const QString& passwordFilename);
    void setFailFilename(const QString& failFilename);
    void setDevice(const QString& device);
    void setProgress(bool showProgress);

public slots:
    void run();

signals:
    void finished();

private:
    bool tryPassword(const QString& password);
    qint64 passwordCount(const QString& passwordFilename);

    void displayFailProgress(const QString& password);
    void displaySuccessProgress(const QString& password);
    void displaySummary();
    void displayDone();

    QString passwordFilename;
    QString failFilename;
    QString device;
    bool showProgress;

    qint64 totalPasswords;
    qint64 triedCount;

    QElapsedTimer totalTimer;
    QElapsedTimer elapsedTimer;
};

#endif // TRYDICTIONARY_H
