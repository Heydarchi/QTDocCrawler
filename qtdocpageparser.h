#include <QString>
#include <QList>
#include <QRegExp>

#ifndef QTDOCPAGEPARSER_H
#define QTDOCPAGEPARSER_H


class QtDocPageParser
{
public:
    QtDocPageParser(QString, QString);
    QList<QString>  parseHtml();

private:
    QString         content;
    QString         base_url;

};

#endif // QTDOCPAGEPARSER_H
