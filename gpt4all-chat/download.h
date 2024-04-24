#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QVariant>
#include <QTemporaryFile>
#include <QThread>
#include <QMutex>

struct ReleaseInfo {
    QString version;
    QString notes;
    QString contributors;
};

class HashAndSaveFile : public QObject
{
    Q_OBJECT
public:
    explicit HashAndSaveFile(QObject* parent = nullptr);

public Q_SLOTS:
    void hashAndSave(const QString &hash, QCryptographicHash::Algorithm a, const QString &saveFilePath,
                     QFile *tempFile, QNetworkReply *modelReply);

Q_SIGNALS:
    void hashAndSaveFinished(bool success, const QString &error,
                             QFile *tempFile, QNetworkReply *modelReply);

private:
    QThread m_hashAndSaveThread;
};

class Download : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool hasNewerRelease READ hasNewerRelease NOTIFY hasNewerReleaseChanged)
    Q_PROPERTY(ReleaseInfo releaseInfo READ releaseInfo NOTIFY releaseInfoChanged)

public:
    static Download *globalInstance();

    ReleaseInfo releaseInfo() const;
    bool hasNewerRelease() const;
    Q_INVOKABLE void downloadModel(const QString &modelFile);
    Q_INVOKABLE void cancelDownload(const QString &modelFile);
    Q_INVOKABLE void installModel(const QString &modelFile, const QString &apiKey);
    Q_INVOKABLE void removeModel(const QString &modelFile);
    Q_INVOKABLE bool isFirstStart() const;

public Q_SLOTS:
    void updateReleaseNotes();

private Q_SLOTS:
    void handleSslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
    void handleReleaseJsonDownloadFinished();
    void handleErrorOccurred(QNetworkReply::NetworkError code);
    void handleDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void handleModelDownloadFinished();
    void handleHashAndSaveFinished(bool success, const QString &error,
                                    QFile *tempFile, QNetworkReply *modelReply);
    void handleReadyRead();

Q_SIGNALS:
    void releaseInfoChanged();
    void hasNewerReleaseChanged();
    void requestHashAndSave(const QString &hash, QCryptographicHash::Algorithm a, const QString &saveFilePath,
                            QFile *tempFile, QNetworkReply *modelReply);

private:
    void parseReleaseJsonFile(const QByteArray &jsonData);
    QString incompleteDownloadPath(const QString &modelFile);
    bool hasRetry(const QString &filename) const;
    bool shouldRetry(const QString &filename);
    void clearRetry(const QString &filename);

    explicit Download(QObject* parent = nullptr);
    ~Download() {}
    friend class MyDownload;

    QMutex m_mutex; // Mutex for thread safety
    HashAndSaveFile *m_hashAndSave;
    QMap<QString, ReleaseInfo> m_releaseMap;
    QNetworkAccessManager m_networkManager;
    QMap<QNetworkReply*, QFile*> m_activeDownloads;
    QHash<QString, int> m_activeRetries;
    QDateTime m_startTime;
};

#endif // DOWNLOAD_H
