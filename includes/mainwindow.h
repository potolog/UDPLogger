/***
 *  This file is part of UDPLogger
 *
 *  Copyright (C) 2018 Martin Marmsoler, martin.marmsoler at gmail.com
 *
 *  UDPLogger is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with UDPLogger.  If not, see <http://www.gnu.org/licenses/>.
 ***/

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
    void writeSettings();
    void readSettings();
 public Q_SLOTS:
    void disableStartUDP();
    void disableStopUDP();
    void changeDockVisibility(bool checked);
    void changedDockVisibility(bool visible);


private:
    Ui::MainWindow *ui;
    //QUdpSocket *m_socket;
    QVBoxLayout *m_vlayout;
    Plots *m_plots;
    QVector<double> qv_x, qv_y;
    Signals* m_signal;
    QAction* m_start_udp;
    QAction* m_stop_udp;
    QAction* m_show_trigger_dock;
    QDockWidget* m_trigger_menu;
};

#endif // MAINWINDOW_H
