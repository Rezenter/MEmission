#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Me emission monitor");
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
    QObject::connect(port, SIGNAL(error(QString)), log, SLOT(error(QString)));
    QObject::connect(port, SIGNAL(out(QString)), log, SLOT(received(QString)));
    QObject::connect(ui->connectButton, &QPushButton::pressed, this, &MainWindow::connectButton);
    QObject::connect(ui->clearButton, &QPushButton::pressed, this, &MainWindow::clearButton);
    QObject::connect(ui->spBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, &MainWindow::changed);
    QObject::connect(ui->spBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::finished);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(watchdog()));
    ui->statusLabel->setStyleSheet("QLabel { background-color : red; color : blue; }");
    chart = new QtCharts::QChart();
    axisX = new QtCharts::QDateTimeAxis();
    axisX->setFormat("H m s");
    axisX->setTitleText("Date");
    axisX->setMinorGridLineVisible(true);
    axisY = new QtCharts::QValueAxis();
    axisY->setMinorGridLineVisible(true);
    axisP = new QtCharts::QValueAxis();
    axisP->setMinorGridLineVisible(true);
    axisP->setMax(100);
    axisP->setMin(0);
    axisY->setMin(0);
    axisY->setMax(65);
    axisP->setTitleText("Power, %");
    axisY->setTitleText("Current, mA");
    c = new QtCharts::QLineSeries();
    p = new QtCharts::QLineSeries();
    sps = new QtCharts::QLineSeries();
    chart->addSeries(p);
    chart->addSeries(c);
    chart->addSeries(sps);
    chartView = new QtCharts::QChartView(chart, ui->chartWidget);
    chartView->setRenderHint(QPainter::Antialiasing);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisP,Qt::AlignRight);
    chart->addAxis(axisY, Qt::AlignLeft);
    ui->chartWidget->resize(439, 349);
    p->attachAxis(axisP);
    c->attachAxis(axisY);
    sps->attachAxis(axisY);
    p->attachAxis(axisX);
    c->attachAxis(axisX);
    sps->attachAxis(axisX);
    p->setName("Power");
    c->setName("HV Current");
    sps->setName("SetPoint Current");
    p->setColor(QColor(255, 0, 0));
    c->setColor(QColor(0, 255, 0));
    sps->setColor(QColor(0, 0, 255));
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
    p->clear();
    c->clear();
    sps->clear();
    axisX->setMin(now.currentDateTime());
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
    opened = false;
    timer->stop();
    log->print("port closed");
}

void MainWindow::error(QString data){
    if(QString::compare("No such file or directory", data, Qt::CaseSensitive) == 0){
        data = "No such port, file or directory. I/O open error.";
    }
    qDebug() << "error: " << data;
}

void MainWindow::in(QString data){
    data.chop(2);
    if(data.endsWith("connected", Qt::CaseInsensitive)){
         ui->statusLabel->setStyleSheet("QLabel { background-color : green; color : black; }");
         opened = true;
         timer->start(100);
         now.currentDateTime();
         axisX->setMin(now.currentDateTime());
    }else{
        QStringList params = data.split(' ');
        if(params.length() == 4){
            ui->cLabel->setText(params.at(0));
            ui->pLabel->setText(params.at(1));
            ui->spLabel->setText(params.at(2));
            int err = params.at(3).toInt();
            if(err == -1 || err == -3 || err == -5 || err == -7){
                error("no watchdog");
            }
            if(err == -2 || err == -3 || err == -6 || err == -7){
                ui->pLabel->setStyleSheet("QLabel { background-color : red; color : black; }");
                error("power lost");
            }
                if(err == -4 || err == -5 || err == -6 || err == -7){
                ui->spBox->setStyleSheet("QDoubleSpinBox { background-color : red; color : black; }");
                error("wrong SP = " + QString::number(sp));
                sp = 0;
                ui->spBox->setValue(0);
            }
            if(err == 0){
                ui->statusLabel->setStyleSheet("QLabel { background-color : green; color : black; }");
            }else{
                ui->statusLabel->setStyleSheet("QLabel { background-color : yellow; color : black; }");
            }
            if(p->points().length() >= 36000){
                p->remove(0);
                c->remove(0);
                sps->remove(0);
                axisX->setMin(QDateTime::fromMSecsSinceEpoch(p->at(0).x()));
            }
            p->append(now.currentDateTime().toMSecsSinceEpoch(), params.at(1).toDouble());
            c->append(now.currentDateTime().toMSecsSinceEpoch(), params.at(0).toDouble());
            sps->append(now.currentDateTime().toMSecsSinceEpoch(), params.at(2).toDouble());
            axisX->setMax(now.currentDateTime());
        }else{
            error(data);
        }
    }
}

void MainWindow::watchdog(){
    emit send(QString::number(sp));
}

void MainWindow::resizeEvent(QResizeEvent* event){
    QMainWindow::resizeEvent(event);
    chartView->resize(ui->chartWidget->size());

}
