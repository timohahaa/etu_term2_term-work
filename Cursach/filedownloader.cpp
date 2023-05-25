#include "filedownloader.h"
#include <QFile>
#include <QDebug>
#include <QEventLoop>

//https://hosp15.com/doc/medrabotniki.xlsx
FileDownloader::FileDownloader(QObject *parent) : QObject(parent) {
    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
}

void FileDownloader::download(QString urlString, QString pathToSaveTo) {
    QUrl url(urlString);
    QNetworkRequest request;
    request.setUrl(url);
    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(readyRead()), this, SLOT(readyToRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(progress(qint64,qint64)));
    connect(reply, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(err(QNetworkReply::NetworkError)));
    setFilePathToSaveTo(pathToSaveTo);
    loop.exec();
}

void FileDownloader::setFilePathToSaveTo(const QString &newFilePathToSaveTo) {
    filePathToSaveTo = newFilePathToSaveTo;
}

int FileDownloader::getLastError() {
    return lastError;
}

void FileDownloader::setLastError(int newLastError) {
    lastError = newLastError;
    //0 means all is ok, 1 means an error occured
}

void FileDownloader::downloadFinished(QNetworkReply *reply) {
    QByteArray fileBytes = reply->readAll();
    QFile file(this->filePathToSaveTo);
    if (!file.open(QIODevice::ReadWrite)) {
        setLastError(1);
        return;
    }
    //QDataStream out(&file);
    //out << fileBytes;
    file.write(fileBytes);
    reply->deleteLater();
}

void FileDownloader::readyToRead() {
    QNetworkReply *rep = qobject_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "downloading file...";
}

void FileDownloader::progress(qint64 bytesReceived, qint64 bytesTotal) {
    QNetworkReply *rep = qobject_cast<QNetworkReply*>(QObject::sender());
    qDebug() << "progress: " << bytesReceived << " out of " << bytesTotal;
}

void FileDownloader::err(QNetworkReply::NetworkError code) {
    qDebug() << "error occured: " << code;
}
