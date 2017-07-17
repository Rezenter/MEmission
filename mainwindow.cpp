#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    settings = new QSettings(path + "/settings.ini", QSettings::IniFormat);
    log = new Logger();
    log->print(path);
    comThread = new QThread();
    port = new ComChatter();
    port->moveToThread(comThread);
    port->port.moveToThread(comThread);
    QObject::connect(port, SIGNAL(dead()), comThread, SLOT(quit()));
    QObject::connect(comThread, SIGNAL(finished()), port, SLOT(deleteLater()));
    QObject::connect(port, SIGNAL(dead()), comThread, SLOT(deleteLater()));
    QObject::connect(this, SIGNAL(connect(QString)), port, SLOT(connect(QString)));
    QObject::connect(this, SIGNAL(close()), port, SLOT(close()));
    QObject::connect(this, SIGNAL(send(QString)), port, SLOT(send(QString)));
    QObject::connect(port, SIGNAL(closed()), this, SLOT(closed()));
    QObject::connect(port, SIGNAL(error(QString)), this, SLOT(error(QString)));
    QObject::connect(port, SIGNAL(out(QString)), this, SLOT(in(QString)));
    QObject::connect(this, SIGNAL(send(QString)), log, SLOT(sending(QString)));
    QObject::connect(this, SIGNAL(connect(QString)), log, SLOT(sending(QString)));
    QObject::connect(port, SIGNAL(error(QString)), log, SLOT(error(QString)));
    QObject::connect(port, SIGNAL(out(QString)), log, SLOT(received(QString)));
    QObject::connect(ui->connectButton, &QPushButton::pressed, this, &MainWindow::connectButton);
    QObject::connect(ui->spBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MainWindow::changed);
    QObject::connect(ui->spBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::finished);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(watchdog()));
    ui->statusLabel->setStyleSheet("QLabel { background-color : red; color : blue; }");
    chart = new QtCharts::QChart();
    axisX = new QtCharts::QValueAxis;
    axisX->setMinorGridLineVisible(true);
    chart->setTitle("Evolution");
    axisY = new QtCharts::QValueAxis();
    axisY->setMinorGridLineVisible(true);
    chartView = new QtCharts::QChartView(chart, ui->chartWidget);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->show();
    comThread->start();
}

MainWindow::~MainWindow()
{
    comThread->quit();
    comThread->requestInterruption();
    log->print("QThread dead ? : ");
    if(comThread->wait()){
        log->print("True");
    }else{
        log->print("False");
    }
    delete comThread;
    delete ui;
}

void MainWindow::connectButton(){
    emit close();
    timer->stop();
    if(opened){
        opened = false;
    }else{
        emit connect(settings->value("portName", "COM1").toString());
    }
}

void MainWindow::clearButton(){

}

void MainWindow::changed(double d){
    Q_UNUSED(d);
    ui->spBox->setStyleSheet("QDoubleSpinBox { background-color : yellow; color : black; }");
}

void MainWindow::finished(){
    ui->spBox->setStyleSheet("QDoubleSpinBox { background-color : white; color : black; }");
    sp = ui->spBox->value();
}

void MainWindow::closed(){
    ui->statusLabel->setStyleSheet("QLabel { background-color : red; color : black; }");
}

void MainWindow::error(QString data){
    qDebug() << "error: " << data;
}

void MainWindow::in(QString data){
    data.chop(2);
    if(data.endsWith("connected", Qt::CaseInsensitive)){
         ui->statusLabel->setStyleSheet("QLabel { background-color : green; color : black; }");
         opened = true;
         timer->start(100);
    }else{
        QStringList params = data.split(' ');
        if(params.length() == 3){
            ui->cLabel->setText(params.at(0));
            ui->pLabel->setText(params.at(1));
            ui->spLabel->setText(params.at(2));
        }else{
            if(QString::compare("-1", data, Qt::CaseInsensitive) == 0){
                //no watchdog
            }else{
                if(QString::compare("-2", data, Qt::CaseInsensitive) == 0){
                    ui->pLabel->setStyleSheet("QLabel { background-color : red; color : black; }");
                    //power lost
                }else{
                    if(QString::compare("-3", data, Qt::CaseInsensitive) == 0){
                        ui->spBox->setStyleSheet("QDoubleSpinBox { background-color : red; color : black; }");
                        //wrong sp
                    }
                }
            }
        }
    }
    qDebug() << data;
}

void MainWindow::watchdog(){
    emit send(QString::number(sp));
}
