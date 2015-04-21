#ifndef TRYDICTIONARY_H
#define TRYDICTIONARY_H

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

public slots:
    void run();

signals:
    void finished();

private:
    bool tryPassword(const QString& password);
    qint64 passwordCount(const QString& passwordFilename);

    QString passwordFilename;
    QString failFilename;
    QString device;
};

#endif // TRYDICTIONARY_H
