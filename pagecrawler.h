#include <QQueue>
#include <QThread>
#include <QMutex>
#include <QList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>


#ifndef PAGECRAWLER_H
#define PAGECRAWLER_H


class PageCrawler : public QObject
{


public:
    QNetworkReply           *reply;
    PageCrawler(int);
    PageCrawler(QString, bool=true);
    void                    startCrawl();
    static int              getMainQueueSize();
    static int              getDoneQueueSize();
    static void             saveQueueToFile();
    QString                 extractBaseUrlOfPage(QString);

public slots:
    void                    pageParser();


private :
    int                     thread_id;
    static QQueue<QString>  queue_main;
    static QList<QString>   queue_done;
    static QList<QString>   queue_failed;
    static QList<QString>   queue_base_url;
    static QMutex           mutex_main;

    QString                 current_web_link;
    QString                 base_url;
    static QString          base_domain_url;
    static bool             enable_reject_domain;

    void                    pushMainQueue(QString);
    void                    pushMainQueueAll(QList<QString>);
    void                    pushDoneQueue(QString);
    void                    pushFailedQueue(QString);
    void                    pushBaseUrlQueue(QString);
    static QString          popMainQueue();
    static QString          popDoneQueue();
    static QString          popFailedQueue();
    static QString          popBaseUrlQueue();
    void                    popFromDoneToFailedQueue(QString);
    static void             saveToFile(QString, QString, int);
    static QString          correctFileName(QString);
    void                    checkBaseUrlExist(QString);
    static void             enableBaseDomainUrl(QString, bool);

};

#endif // PAGECRAWLER_H
