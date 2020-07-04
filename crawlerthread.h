#include <QThread>
#include <QtCore>

#ifndef CRAWLERTHREAD_H
#define CRAWLERTHREAD_H


class CrawlerThread : public QThread
{
    Q_OBJECT
public:
    CrawlerThread(QObject *parent = 0);
    // overriding the QThread's run() method
    void        run() override;

protected:
    int         thread_id;
    static int  thread_cnt;
};

#endif // CRAWLERTHREAD_H
