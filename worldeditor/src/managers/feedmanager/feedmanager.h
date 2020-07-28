#ifndef FEEDMANAGER_H
#define FEEDMANAGER_H

#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

class FeedManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString blogFeed READ blogFeed NOTIFY blogFeedChanged)

public:
    explicit FeedManager(QObject *parent = nullptr);

    QString blogFeed () const;

signals:
    void blogFeedChanged();

private slots:
    void replyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_pManager;

    QString m_BlogData;
};

#endif // FEEDMANAGER_H
