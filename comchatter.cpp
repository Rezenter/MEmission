#include "comchatter.h"
#include <QDebug>

ComChatter::ComChatter(QObject *parent) : QObject(parent){

}

ComChatter::~ComChatter(){
    port.waitForBytesWritten(1000);
    close();
    emit dead();
}

void ComChatter::connect(QString name){
    port.setPortName(name);
    port.setBaudRate(QSerialPort::Baud9600);
    port.setDataBits(QSerialPort::Data8);
    port.setParity(QSerialPort::NoParity);
    port.setStopBits(QSerialPort::OneStop);
    port.setFlowControl(QSerialPort::NoFlowControl);
    QObject::connect(&port, SIGNAL(readyRead()), this, SLOT(read()));
    if (port.open(QIODevice::ReadWrite)) {
        if (port.isOpen()){
            QString ping = "ping";
            port.write(ping.toLocal8Bit() + "\n");
        }else{
            emit error(port.errorString());
        }
    }else{
        emit error(port.errorString());
    }
}

void ComChatter::close(){
    if(port.isOpen()){
        port.close();
    }
    emit closed();
}

void ComChatter::send(QString data){
    if(!port.isOpen()){
        emit closed();
    }
    port.write(data.toLocal8Bit());
    port.waitForBytesWritten(10);
}

void ComChatter::read(){
    port.waitForReadyRead(10);
    while(port.bytesAvailable() >= 20){
        emit out(QString(port.readLine()));
    }
}
