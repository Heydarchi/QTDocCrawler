#include <QCoreApplication>
#include <QThread>
#include <QList>
#include "pagecrawler.h"
#include "crawlerthread.h"
#include <QtCore>
#include <QArrayData>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    PageCrawler page_crawler("https://doc.qt.io/qt-5/classes.html");
    page_crawler.startCrawl();

    CrawlerThread list_thread[QThread::idealThreadCount()*4];


    qDebug()<<"Thread number : "<< QThread::idealThreadCount()*4;

    for(int i =0;i<QThread::idealThreadCount()*4;i++ )
        list_thread[i].start();

    for(int i =0;i<QThread::idealThreadCount()*4;i++ )
        list_thread[i].wait();


    qDebug()<<" Main Queue size :"<<PageCrawler::getMainQueueSize() ;
    qDebug()<<" Done Queue size :"<<PageCrawler::getDoneQueueSize() ;
    PageCrawler::saveQueueToFile();
    qDebug()<<" Queues are saved to files" ;
    qDebug()<<" Application Finished !!!" ;

    return a.exec();
}





