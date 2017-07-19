#include "logger.h"

Logger::Logger(QObject *parent) : QObject(parent){
    log = new QFile(filename);
    log->open(QIODevice::WriteOnly);
    stream = new QTextStream(log);
    print(now.currentDateTime().toString("ddd MMMM d yyyy"));
    QDateTime now();
}

Logger::~Logger(){
    log->close();
    delete log;
    delete stream;
}

void Logger::print(QString data){
    QString tmp = '[' + now.currentDateTime().toString("hh:mm:ss") + "] " + data;
    *stream << tmp << endl;
    emit logEvent(tmp);
}

void Logger::error(QString data){
    QString tmp = '[' + now.currentDateTime().toString("hh:mm:ss") + "] error : " + data;
    *stream << tmp << endl;
    emit logEvent(tmp);
}

void Logger::received(QString data){
    QStringList commands = data.split("\r\n", QString::SkipEmptyParts);
    for (auto line : commands){
        QString tmp = '[' + now.currentDateTime().toString("hh:mm:ss") + "] received : " + line;
        *stream << tmp << endl;
        emit logEvent(tmp);
    }
}
