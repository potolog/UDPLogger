#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plots.h"

#include <iostream>
#include "udp.h"
#include "signals.h"

using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_vlayout = new QVBoxLayout(this);

    m_signal = new Signals;

    m_plots = new Plots(ui->centralWidget,m_signal);

    m_vlayout->addWidget(m_plots);
    ui->centralWidget->setLayout(m_vlayout);

    QAction *addPlot = new QAction(tr("&Add Plot"), this);
    addPlot->setStatusTip(tr("Wizard to create a new Plot"));
    connect(addPlot, &QAction::triggered, m_plots, &Plots::createNewPlot);
    ui->menuBar->addAction(addPlot);

    QMenu* menu = ui->menuBar->addMenu(tr("&File"));

    QAction *export_settings = new QAction(tr("&export Settings"), this);
    export_settings->setStatusTip(tr("Wizard to create a new Plot"));
    connect(export_settings, &QAction::triggered, m_plots, &Plots::exportSettings);
    menu->addAction(export_settings);

    QAction *import_settings = new QAction(tr("&import Settings"), this);
    import_settings->setStatusTip(tr("Wizard to create a new Plot"));
    connect(import_settings, &QAction::triggered, m_plots, &Plots::importSettings);
    menu->addAction(import_settings);

    QAction *start_udp = new QAction(tr("Start UDP"), this);
    start_udp->setStatusTip(tr("Starts reading from UDP Buffer"));
    connect(start_udp, &QAction::triggered, m_plots, &Plots::startUDP);
    ui->menuBar->addAction(start_udp);

    QAction *stop_udp = new QAction(tr("Stop UDP"), this);
    stop_udp->setStatusTip(tr("Stops reading from UDP Buffer"));
    connect(stop_udp, &QAction::triggered, m_plots, &Plots::stopUDP);
    ui->menuBar->addAction(stop_udp);

    QAction *settings_widget = new QAction(tr("Settings"), this);
    connect(settings_widget, &QAction::triggered, m_plots, &Plots::settings);
    ui->menuBar->addAction(settings_widget);


}

MainWindow::~MainWindow()
{
    delete m_signal;
    delete ui;

}
