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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plots.h"

#include <iostream>
#include "udp.h"
#include "signals.h"
#include <QDockWidget>
#include "triggerwidget.h"

using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_vlayout = new QVBoxLayout(this);

    m_signal = new Signals;

    m_trigger_menu = new QDockWidget(tr("Store signals to file trigger"),this);
    connect(m_trigger_menu, &QDockWidget::visibilityChanged, this,&MainWindow::changedDockVisibility);
    TriggerWidget* triggerwidget = new TriggerWidget(m_signal,m_trigger_menu);
    connect(m_signal, &Signals::signalsChanged, triggerwidget,&TriggerWidget::updateSignals);
    m_trigger_menu->setWidget(triggerwidget);
    addDockWidget(Qt::RightDockWidgetArea,m_trigger_menu);
    m_trigger_menu->show();

    readSettings();

    m_plots = new Plots(ui->centralWidget,m_signal, triggerwidget);
    m_vlayout->addWidget(m_plots);
    ui->centralWidget->setLayout(m_vlayout);

    QMenu* menu = ui->menuBar->addMenu(tr("&File"));

    QAction *export_settings = new QAction(tr("&export Settings"), this);
    export_settings->setStatusTip(tr("Wizard to create a new Plot"));
    connect(export_settings, &QAction::triggered, m_plots, &Plots::exportSettings);
    menu->addAction(export_settings);

    QAction *import_settings = new QAction(tr("&import Settings"), this);
    import_settings->setStatusTip(tr("Wizard to create a new Plot"));
    connect(import_settings, &QAction::triggered, m_plots, &Plots::importSettings);
    menu->addAction(import_settings);

    QAction *import_signals = new QAction(tr("&import Signals"), this);
    import_signals->setStatusTip(tr("Import new Signals from file"));
    connect(import_signals, &QAction::triggered, m_signal, &Signals::importSignals);
    menu->addAction(import_signals);

    QAction *export_function = new QAction(tr("Export C/C++ Package function"), this);
    connect(export_function, &QAction::triggered, m_signal, &Signals::exportUDPFunction);
    menu->addAction(export_function);

    QAction *addPlot = new QAction(tr("&Add Plot"), this);
    addPlot->setStatusTip(tr("Wizard to create a new Plot"));
    connect(addPlot, &QAction::triggered, m_plots, &Plots::createNewPlot);
    ui->menuBar->addAction(addPlot);

    m_start_udp = new QAction(tr("Start UDP"), this);
    m_start_udp->setStatusTip(tr("Starts reading from UDP Buffer"));
    connect(m_start_udp, &QAction::triggered, m_plots, &Plots::startUDP);
    connect(m_start_udp, &QAction::triggered, this, &MainWindow::disableStartUDP);
    ui->menuBar->addAction(m_start_udp);

    m_stop_udp = new QAction(tr("Stop UDP"), this);
    m_stop_udp->setStatusTip(tr("Stops reading from UDP Buffer"));
    connect(m_stop_udp, &QAction::triggered, m_plots, &Plots::stopUDP);
    connect(m_stop_udp, &QAction::triggered, this, &MainWindow::disableStopUDP);
    ui->menuBar->addAction(m_stop_udp);

    QAction *settings_widget = new QAction(tr("Settings"), this);
    connect(settings_widget, &QAction::triggered, m_plots, &Plots::settings);
    ui->menuBar->addAction(settings_widget);

    // Begin Menu "View"
    QMenu* view_menu = ui->menuBar->addMenu(tr("View"));

    m_show_trigger_dock = new QAction(tr("Trigger dock"),this);
    m_show_trigger_dock->setCheckable(true);
    m_show_trigger_dock->setChecked(true);
    view_menu->addAction(m_show_trigger_dock);
    connect(m_show_trigger_dock, &QAction::toggled, this, &MainWindow::changeDockVisibility);

    disableStopUDP();
}

void MainWindow::disableStartUDP(){
    m_start_udp->setEnabled(false);
    m_stop_udp->setEnabled(true);
}

void MainWindow::disableStopUDP(){
    m_start_udp->setEnabled(true);
    m_stop_udp->setEnabled(false);
}

void MainWindow::writeSettings(){
    QSettings settings("Murmele", "com.github.Murmele.UDPLogger");

    settings.beginGroup("mainWindow");
    settings.setValue("geometry",saveGeometry());
    settings.setValue("state", saveState());
    settings.endGroup();
}

void MainWindow::readSettings(){
    QSettings settings("Murmele", "com.github.Murmele.UDPLogger");
    settings.beginGroup("mainWindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
    settings.endGroup();
}

MainWindow::~MainWindow()
{
    writeSettings();
    delete m_plots;
    delete m_signal;
    delete ui;

}

void MainWindow::changeDockVisibility(bool checked){
    Q_UNUSED(checked);
    m_trigger_menu->setVisible(m_show_trigger_dock->isChecked());

}

void MainWindow::changedDockVisibility(bool visible){
    Q_UNUSED(visible);
    m_show_trigger_dock->setChecked(m_trigger_menu->isVisible());
}
