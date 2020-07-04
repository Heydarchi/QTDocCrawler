#include "crawlerthread.h"
#include "pagecrawler.h"

int  CrawlerThread::thread_cnt;

CrawlerThread::CrawlerThread(QObject *parent): QThread(parent)
{
    thread_id=++thread_cnt;
}


void CrawlerThread::run()
{
    while(PageCrawler::getMainQueueSize()>0)
    {
        PageCrawler page_crawler(thread_id);
        page_crawler.startCrawl();
    }
}
