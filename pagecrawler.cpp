#include "pagecrawler.h"
#include "qtdocpageparser.h"
#include <QUrl>
#include <QRegExp>
#include <QDebug>
#include <QEventLoop>
#include <QIODevice>
#include <QFile>
#include <QDir>

QQueue<QString> PageCrawler::queue_main;
QList<QString>  PageCrawler::queue_done;
QList<QString>  PageCrawler::queue_failed;
QList<QString>  PageCrawler::queue_base_url;
QMutex          PageCrawler::mutex_main;
bool            PageCrawler::enable_reject_domain;
QString         PageCrawler::base_domain_url;

PageCrawler::PageCrawler(int _thread_id)
{
    thread_id=_thread_id;
    current_web_link=popMainQueue();
    base_url=extractBaseUrlOfPage(current_web_link);
    pushDoneQueue(current_web_link);
}

PageCrawler::PageCrawler(QString _web_link, bool _reject_en)
{
    thread_id=0;
    current_web_link=_web_link;
    base_url=extractBaseUrlOfPage(current_web_link);
    pushDoneQueue(current_web_link);
    enableBaseDomainUrl(_web_link, _reject_en);
}


/**
 * Fetch page content of the address which the link points to
 *
*/
void PageCrawler::startCrawl()
{
    if(current_web_link.isEmpty())
        return;

    if( enable_reject_domain && !current_web_link.contains(base_domain_url) )
    {
        qDebug()<<"Thread : "<< thread_id <<" Not in domain so rejected : "<<current_web_link;
        return;
    }

    QUrl url(current_web_link);

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.10; rv:31.0) Gecko/20100101 Firefox/31.0");
    qDebug()<<"Thread : "<< thread_id <<" request : "<<request.url();

    QEventLoop loop;
    QNetworkAccessManager access_manager;
    reply=access_manager.get(request);
    QObject::connect( reply, &QNetworkReply::finished, &loop, &QEventLoop::quit  );
    loop.exec();

    //Parse fetched content
    pageParser();
    reply->close();
    access_manager.disconnect();
    //qDebug()<<"Base Url : "<<extractBaseUrlOfPage( current_web_link );
}


/**
 * By calling this function the retrive content from the link will be parsed
 *
*/
void PageCrawler::pageParser()
{
    if(reply->error()==QNetworkReply::NoError)
    {
        QString reply_str=QString::fromUtf8(reply->readAll());

        //Check if the path exists
        checkBaseUrlExist( base_url );

        //Save content to file
        saveToFile(current_web_link,reply_str, thread_id);

        //Parse content to find all the links being inside the content
        QtDocPageParser qt_doc_parser(reply_str, base_url );
        QList<QString> list_links=qt_doc_parser.parseHtml();

        //insert all links which are found into queue
        pushMainQueueAll(list_links);
        //qDebug()<<" Links count : "<<list_links.size();
    }
    else
    {
        saveToFile(current_web_link,(QString)reply->error(), thread_id);
        qDebug()<<"reply"<<reply<<reply->error();
        popFromDoneToFailedQueue(current_web_link);
    }

}

/**
 * Save the content to file
 *
 * @_content QString type which includes content which must be saved
 * @_fn QString type including file name
*/
void PageCrawler::saveToFile(QString _fn,QString _content, int _thread_id=100)
{

    //check to replace all invalid letters inside the file name
    QString filename=correctFileName(_fn);

    //qDebug()<<"Thread : "<< _thread_id << " File name : "<<filename;

    //Save content to file
    QFile file( filename );
    if ( file.open(QIODevice::ReadWrite|QIODevice::Truncate) )
    {
        QTextStream stream( &file );
        stream <<_content << endl;
    }
}

/**
 * Save Queue content to file
 *
*/
void PageCrawler::saveQueueToFile()
{
    QString all_cnt;
    QString q_cnt;


    //Save Main Queue to File
    do{
        q_cnt=popMainQueue();
        all_cnt+=q_cnt+"\n";
    }while (  q_cnt!=NULL );
    saveToFile("QueueMain.txt",all_cnt);

    //Save Done Queue to File
    all_cnt.clear();
    do{
        q_cnt=popDoneQueue();
        all_cnt+=q_cnt+"\n";
    }while ( q_cnt!=NULL ) ;
    saveToFile("QueueDone.txt",all_cnt);

    //Save Failed Queue to File
    all_cnt.clear();
    do{
        q_cnt=popFailedQueue();
        all_cnt+=q_cnt+"\n";
    }while ( q_cnt!=NULL ) ;
    saveToFile("QueueFailed.txt",all_cnt);

    //Save Base Url Queue to File
    all_cnt.clear();
    do{
        q_cnt=popBaseUrlQueue();
        all_cnt+=q_cnt+"\n";
    }while ( q_cnt!=NULL ) ;
    saveToFile("QueueBaseUrl.txt",all_cnt);
}

/**
 * Replace the invalid letters inside the file name string
 *
 * @return QString type returning corrected file name
*/
QString PageCrawler::correctFileName(QString _fn)
{
    _fn=_fn.replace("https://","").replace("http://","");

    if( _fn.lastIndexOf("/")==(_fn.length()-1))
        _fn+="index.html";

    #ifdef Windows
        QStringList invalid_latters={":","/","&"};
        QString dir_seprator="\\";
    #else
        QStringList invalid_latters={":","\\","&"};
        QString dir_seprator="/";
    #endif

    foreach (QString _letter, invalid_latters) {
        _fn=_fn.replace(_letter,"_")   ;
    }


    return _fn;
}

/**
 * Parse the Page url to find Base Url without page name
 *
 * @_url QString type includes address which must be  parsed
 * @return QString type returning base address
*/
QString PageCrawler::extractBaseUrlOfPage(QString _url)
{
    return ( _url.left(_url.lastIndexOf("/"))+"/" );
}

/**
 * Parse the Url to find the base domain
 *
 * @_url QString type includes address which must be parsed
*/
void PageCrawler::enableBaseDomainUrl(QString _url, bool _enable_rejects)
{
    enable_reject_domain=_enable_rejects;

    QStringList list_name=_url.replace("https://","").replace("http://","").split("/");
    QStringList list_domain_name=list_name[0].split(".");

    if(list_domain_name.length()==2)
        base_domain_url=list_domain_name.join(".");
    else if(list_domain_name.length()>2)
        base_domain_url=list_domain_name[list_domain_name.length()-2] + "." + list_domain_name[list_domain_name.length()-1];

    qDebug()<<"  base_domain_url : "<<base_domain_url;

}


/**
 * Check if the path exists and create the sub-folders if it is required
 *
 * @_url QString type includes path which must be checkes
*/
void PageCrawler::checkBaseUrlExist(QString _url)
{
    #ifdef Windows
        QString dir_seprator="\\";
    #else
        QString dir_seprator="/";
    #endif

    QStringList url_split= _url.replace("https://","").replace("http://","").split("/");
    QString path;
    QString dir;
    foreach( dir, url_split)
    {
        if(!dir.trimmed().isEmpty())
        {
            path+=dir+dir_seprator;

            //Create path if it does not exist in queue of created paths
            if( !queue_base_url.contains(path) )
            {
                QDir make_dir(path);
                make_dir.mkpath(".");
                pushBaseUrlQueue(path);
            }
        }
    }
}

int  PageCrawler::getMainQueueSize()
{
    return queue_main.size();
}

int  PageCrawler::getDoneQueueSize()
{
    return queue_done.size();
}


void PageCrawler::pushMainQueueAll(QList<QString> _web_link_list)
{
    mutex_main.lock();
    foreach(QString _web_link , _web_link_list)
        if(!queue_main.contains(_web_link) && !queue_done.contains(_web_link) && !queue_failed.contains(_web_link))
            queue_main.enqueue(_web_link);
    mutex_main.unlock();
}

void PageCrawler::pushMainQueue(QString _web_link)
{
    mutex_main.lock();
    if(!queue_main.contains(_web_link) && !queue_done.contains(_web_link) && !queue_failed.contains(_web_link))
        queue_main.enqueue(_web_link);
    mutex_main.unlock();
}


void PageCrawler::pushDoneQueue(QString _web_link)
{
    mutex_main.lock();
    queue_done.push_back(_web_link);
    mutex_main.unlock();
}

void PageCrawler::pushFailedQueue(QString _web_link)
{
    mutex_main.lock();
    queue_failed.push_back(_web_link);
    mutex_main.unlock();
}

void PageCrawler::pushBaseUrlQueue(QString _web_link)
{
    if(queue_base_url.contains( _web_link ) )
        return;
    mutex_main.lock();
    queue_base_url.push_back(_web_link);
    mutex_main.unlock();
}

QString PageCrawler::popMainQueue()
{
    if(queue_main.isEmpty())
        return NULL;

    mutex_main.lock();
    QString _web_link=queue_main.dequeue();
    mutex_main.unlock();
    return _web_link;
}


QString PageCrawler::popDoneQueue()
{
    if(queue_done.isEmpty())
        return NULL;

    mutex_main.lock();
    QString _web_link=queue_done.last();
    queue_done.removeLast();
    mutex_main.unlock();
    return _web_link;
}


QString PageCrawler::popFailedQueue()
{
    if(queue_failed.isEmpty())
        return NULL;

    mutex_main.lock();
    QString _web_link=queue_failed.last();
    queue_failed.removeLast();
    mutex_main.unlock();
    return _web_link;
}

QString PageCrawler::popBaseUrlQueue()
{
    if(queue_base_url.isEmpty())
        return NULL;

    mutex_main.lock();
    QString _web_link=queue_base_url.last();
    queue_base_url.removeLast();
    mutex_main.unlock();
    return _web_link;
}

void PageCrawler::popFromDoneToFailedQueue(QString _web_link)
{
    mutex_main.lock();
    queue_done.removeOne(_web_link);
    queue_failed.push_back(_web_link);
    mutex_main.unlock();
}


