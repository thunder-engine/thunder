#include "feedmanager.h"

#include <QFile>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

FeedManager::FeedManager(QObject *parent) :
        QObject(parent) {

    m_pManager = new QNetworkAccessManager(this);
    connect(m_pManager, &QNetworkAccessManager::finished,
            this, &FeedManager::replyFinished);

    QNetworkRequest req(QUrl("http://thunderengine.org/feed.xml"));
    req.setRawHeader("Accept", "application/xml,*/*");

    m_pManager->get(req);
}

QString FeedManager::blogFeed() const {
    return m_BlogData;
}

void FeedManager::replyFinished(QNetworkReply *reply) {
    m_BlogData = reply->readAll();
    emit blogFeedChanged();
}
