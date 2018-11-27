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

#include "plots.h"
#include <QDebug>
#include "udp.h"
#include "signals.h"
#include <QFileDialog>
#include <QMessageBox>
#include "settingsdialog.h"
#include "plot.h"
#include "changegraphdialog.h"
#include "triggerwidget.h"

Plots::Plots(QWidget *parent, Signals* signal, TriggerWidget* trigger): m_parent(parent), m_signals(signal)
{
    m_layout = new QVBoxLayout(parent);
    m_index_new_data = 0;
    m_ifudpLogging = false;
    m_use_data_count = 0;

    m_data_buffers = new PlotBuffers(signal);

    m_mutex = new QMutex;
    m_udp = new UDP(this, m_mutex,m_data_buffers, signal, trigger);
    connect(m_udp, &UDP::showInfoMessageBox, this, &Plots::showInfoMessageBox,Qt::ConnectionType::QueuedConnection);



    m_udp->moveToThread(&m_udp_thread);
    m_udp_thread.start();

    connect(this, &Plots::connectToReadyRead, m_udp, &UDP::connectDataReady, Qt::ConnectionType::QueuedConnection);
    connect(this, &Plots::disconnectToReadyRead, m_udp, &UDP::disconnectDataReady, Qt::ConnectionType::QueuedConnection);
    qRegisterMetaType<QHostAddress>("QHostAddress"); // register QHostAddress to be usable in signal/slots
    connect(this, &Plots::initUDP, m_udp, qOverload<QHostAddress, quint16, int,int,int, QString, QString>(&UDP::init), Qt::ConnectionType::QueuedConnection);
    connect(this, &Plots::changeSignalSettings, signal, &Signals::changeSignalSettings);

    connect(this, &Plots::plotBufferSizeChanged, m_data_buffers, &PlotBuffers::plotBufferSizeChanged);


    m_settings_dialog = new SettingsDialog(this);
}

QSharedPointer<QCPGraphDataContainer> Plots::getBuffer(struct Signal xaxis, struct Signal yaxis){

    QSharedPointer<QCPGraphDataContainer> pointer = m_data_buffers->getBuffer(xaxis, yaxis);
    return pointer;
}

void Plots::removeGraph(struct Signal xaxis, struct Signal yaxis){
    m_data_buffers->removeGraph(xaxis, yaxis);
}

void Plots::deletePlot(Plot* plot_address){

    plot_address->deleteLater();

    for(int i=0; i<m_plots.length(); i++){
        if(m_plots.at(i) == plot_address){
            m_plots.remove(i);
            break;
        }
    }

}

void Plots::exportSettings(){
    QString select = tr("UDP Logger Config Files (*.udpLoggerSettings)");
    QFileDialog filedialog;
    filedialog.setDefaultSuffix(".udpLoggerSettings");
    QString fileName = filedialog.getSaveFileName(this,
            tr("Export Settings"), "/home",
            select,&select);

    if(fileName.compare("") == 0)
        return;
    QJsonObject object;

    object["ProjectName"] = m_project_name;
    object["ProgramVersion"] = m_program_version;

    QJsonArray plots;
    for (int i=0; i< m_plots.length(); i++){
        QJsonObject plotObject;
        m_plots[i]->writeJSON(plotObject);
        plots.append(plotObject);
     }
    object["Plots"] = plots;


    m_signals->writeToJsonObject(object);

    QJsonObject settings;
    m_settings_dialog->createJSONObject(settings);

    object["Settings"] = settings;


    QFile saveFile(fileName);

   if (!saveFile.open(QIODevice::WriteOnly)) {
       qWarning("Couldn't open save file.");
       //return false;
   }

   QJsonDocument document(object);
   saveFile.write(document.toJson());
   saveFile.close();
}

void Plots::importSettings(){

    if(m_ifudpLogging){
        QMessageBox msgBox;
        msgBox.setText(tr("Please Stop UDP Logging"));
        msgBox.exec();

        return;
    }

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open settingsfile"), "/home", tr("UDP Logger Config Files (*.udpLoggerSettings)"));

	if(fileName.compare("")==0)
		return;

    // before importing remove everything!!!!
    int size = m_plots.size();
    for (int i=0; i<size; i++){
        deletePlot(m_plots.at(0));
    }



    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

    QJsonObject object = d.object();

    // signals
    m_signals->parseJsonObject(object);

    // plot settings
    if(object.contains("Plots")){
        QJsonArray plots = object["Plots"].toArray();

        for(int i=0; i<plots.count(); i++){
            Plot* plot = createNewPlot();

            QJsonObject plot_settings = plots[i].toObject();
            plot->importSettings(plot_settings);
        }
    }

    // global settings
    if(object.contains("Settings") && object.contains("ProjectName")){
        QJsonObject settings;
        settings = object["Settings"].toObject();
        QString project_name = object["ProjectName"].toString();
        m_settings_dialog->readJSONObject(settings, project_name);
    }
    file.close();
}

Plot* Plots::createNewPlot()
{
    Plot* plot = new Plot(this, m_parent, m_plots.length(),m_signals);
    connect(plot, &Plot::removeSignal, m_data_buffers, &PlotBuffers::removeSignal);
    m_plots.append(plot);
    m_layout->addWidget(plot);
    connect(m_udp, &UDP::dataChanged, plot, &Plot::newData);
    connect(m_signals, &Signals::signalsChanged, plot, &Plot::signalsChanged);
    return plot;
}

void Plots::startUDP(){
    emit connectToReadyRead();
    m_ifudpLogging = 1;
}

void Plots::stopUDP(){
    emit disconnectToReadyRead();
    m_ifudpLogging=0;
}

void Plots::settings(){
    if(m_ifudpLogging){
        QMessageBox msgBox;
        msgBox.setText("Please Stop UDP Logging");
        msgBox.exec();

        return;
    }

    m_settings_dialog->exec();
}

void Plots::settingsAccepted(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int port, int refresh_rate, int use_data_count, QString export_path, QString relative_header_path, QString additional_includes){
    m_project_name = project_name;
    m_hostaddress = hostname;
    m_port = port;
    m_refresh_rate = refresh_rate;
    m_use_data_count = use_data_count;
    emit changeSignalSettings(relative_header_path, additional_includes);
    emit plotBufferSizeChanged(plot_buffersize);
    emit initUDP(hostname,static_cast<quint16>(port),udp_buffersize,refresh_rate, use_data_count, export_path,project_name);
}

void Plots::showInfoMessageBox(QString title, QString text){
    QMessageBox msgBox(m_parent);
    msgBox.setWindowTitle(title);
    msgBox.setText(text);
    msgBox.exec();
}

void Plots::clearPlots(){
    m_mutex->lock();
    m_data_buffers->clearPlots();
    m_mutex->unlock();
    for(int i=0; i<m_plots.length();i++){
        m_plots[i]->replot();
    }
}

Plots::~Plots(){

    int size = m_plots.length();
    for(int i=0; i<size; i++){
        deletePlot(m_plots.at(0));
    }
    m_udp_thread.quit();
    m_udp_thread.wait(); // waiting till thread quits
    delete m_udp;
    delete m_mutex;
    delete m_data_buffers;
}
