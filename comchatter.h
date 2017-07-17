#ifndef COMCHATTER_H
#define COMCHATTER_H

#include <QObject>
#include <QtSerialPort/QserialPort>

class ComChatter : public QObject{

    Q_OBJECT

public:
    ComChatter(QObject *parent = 0);
    ~ComChatter();
    QSerialPort port;

signals:
    void closed();
    void error(QString error);
    void out(QString data);
    void dead();



public slots:
    void connect(QString name);
    void close();
    void send(QString data);
    void read();
};

#endif // COMCHATTER_H
