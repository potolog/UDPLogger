#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include "plots.h"

class Signals;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void addPoint(double x, double y);
    void addPlot();
    void clearData();
    void plot();


private:
    Ui::MainWindow *ui;
    //QUdpSocket *m_socket;
    QVBoxLayout *m_vlayout;
    Plots *m_plots;
    QVector<double> qv_x, qv_y;
    Signals* m_signal;




};

#endif // MAINWINDOW_H
