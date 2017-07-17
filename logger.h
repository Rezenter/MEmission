#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QCoreApplication>


class Logger : public QObject{

    Q_OBJECT

public:
    Logger(QObject *parent = 0);
    ~Logger();
    void print(QString data);

private:
    QString filename = QCoreApplication::applicationDirPath() + "/log.txt";
    QFile *log;
    QTextStream *stream;
    QDateTime now;

public slots:
    void sending(QString data);
    void error(QString data);
    void received(QString data);

signals:
    void logEvent(QString data);
};

#endif // LOGGER_H
