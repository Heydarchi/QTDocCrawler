#include "qtdocpageparser.h"
#include <QDebug>

QtDocPageParser::QtDocPageParser(QString _content, QString _base_url)
{
    content=_content;
    base_url=_base_url;
}


/**
 * Find all href inside the page content and returns them
 *
 * @return QList<QString> type which includes the addresses found
*/
QList<QString> QtDocPageParser::parseHtml()
{
    QList<QString>  list_links;
    QString         href_pattern="<a[\\sa-zA-Z':=\"\\/\\.\\-\\_]*href=\"[\\sa-zA-Z':\\/\\.\\-\\_]*\"";
    QRegExp         rx(href_pattern);
    int pos =0;
    while ( ( pos=rx.indexIn(content,pos) ) !=-1 )
    {
        QStringList list = rx.capturedTexts();
        foreach (QString link, list) {
            //Remove extra letter to get address clearly
            link=link.mid(link.indexOf("href=\"")+6).replace("\"","");

            //Add base address if it does not exist
            if(!link.contains("https:") && !link.contains("http:"))
                link=base_url+"/"+ link;

            //Remove duplicated address seprator
            link=link.replace("///","/");
            if( !link.isEmpty() && !list_links.contains(link))
                list_links.push_back( link.replace("///","/") );
            //qDebug()<<link;
        }
        pos+=rx.matchedLength();
    }

    return list_links;
}

