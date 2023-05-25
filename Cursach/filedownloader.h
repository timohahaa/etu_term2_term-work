#include <QObject>

#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>


class FileDownloader : public QObject
{

    Q_OBJECT

private:
    QNetworkAccessManager manager;
    QString filePathToSaveTo;
    //0 is all fine, 1 is error
    int lastError = 0;
    void setFilePathToSaveTo(const QString &newFilePathToSaveTo);
    void setLastError(int newLastError);

private slots:
    void downloadFinished(QNetworkReply *reply);
    void readyToRead();
    void progress(qint64 bytesReceived, qint64 bytesTotal);
    void err(QNetworkReply::NetworkError code);

public:
    FileDownloader(QObject *parent = nullptr);
    void download(QString url, QString pathToSaveTo);
    int getLastError();
};

#endif // FILEDOWNLOADER_H
