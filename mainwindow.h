#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include "comchatter.h"
#include "logger.h"
#include <QCoreApplication>
#include <QSettings>
#include <QChartView>
#include <QValueAxis>
#include <QLineSeries>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void connect(QString);
    void close();
    void send(QString);

public slots:
    void closed();
    void error(QString);
    void in(QString);

private:
    Ui::MainWindow *ui;
    QString path = QCoreApplication::applicationDirPath();
    QSettings *settings;
    bool opened = false;
    QThread *comThread;
    ComChatter *port;
    Logger *log;
    QString portName;
    QtCharts::QChartView *chartView;
    QtCharts::QChart *chart;
    QtCharts::QLineSeries *p;
    QtCharts::QLineSeries *c;
    QtCharts::QValueAxis *axisX;
    QtCharts::QValueAxis *axisY;
    double sp;
    QTimer *timer = new QTimer(this);

private slots:
    void connectButton();
    void clearButton();
    void changed(double);
    void finished();
    void watchdog();
};

#endif // MAINWINDOW_H
