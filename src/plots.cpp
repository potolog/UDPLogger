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

Plots::Plots(QWidget *parent, Signals* signal): m_parent(parent), m_signals(signal)
{
    m_program_version = "1.0";
    m_project_name = "Project1";
    m_layout = new QVBoxLayout(parent);
    m_index_buffer = 0;
    m_ifudpLogging = false;

    int port = 60000;
    int udp_buffersize = 400;
    int plot_buffersize = 200;
    int data_buffersize = 500;
    m_plot_buffersize = plot_buffersize;
    m_udp_buffersize = udp_buffersize;
    m_data_buffersize = data_buffersize;
    m_export_data = false;

    changeDataBufferSize(m_plot_buffersize,m_udp_buffersize);
    m_port = port;
    m_hostaddress = QHostAddress::Any;

    m_mutex = new QMutex;
    m_udp = new UDP(this, m_mutex);
    m_udp->init();

    m_udp->moveToThread(&m_udp_thread);
    m_udp_thread.start();
    //m_ifudpLogging = 1;
    //m_udp->connectDataReady();
    connect(this, &Plots::connectToReadyRead, m_udp, &UDP::connectDataReady, Qt::ConnectionType::QueuedConnection);
    connect(this, &Plots::disconnectToReadyRead, m_udp, &UDP::disconnectDataReady, Qt::ConnectionType::QueuedConnection);
    qRegisterMetaType<QHostAddress>("QHostAddress"); // register QHostAddress to be usable in signal/slots
    connect(this, &Plots::initUDP, m_udp, qOverload<QHostAddress, quint16, int, bool, QString>(&UDP::init), Qt::ConnectionType::QueuedConnection);

    m_settings_dialog = new SettingsDialog(this);
    m_settings_dialog->setSettings(m_project_name, m_hostaddress, m_udp_buffersize, m_plot_buffersize,m_data_buffersize, m_port, m_export_data,"");
}

void Plots::newData(){
    if(m_index_buffer >= m_data_buffersize){
        m_index_buffer = 0;
    }
    m_mutex->lock();
    struct Signal signal;
    for (int i=0; i<m_signals->getSignalCount(); i++){
        signal = m_signals->getSignal(i);

        double value;
        if(signal.datatype.compare("float")== 0){
            float temp =*m_udp->getFloatPointer(signal.offset);
            value = static_cast<double>(temp);

        }else if(signal.datatype.compare("double")==0){
            value = *m_udp->getDoublePointer(signal.offset);

        }else if(signal.datatype.compare("char")==0){
            char temp = *m_udp->getCharPointer(signal.offset);
            value = static_cast<double>(temp);

        }else if(signal.datatype.compare("int")==0){
            int temp = *m_udp->getIntPointer(signal.offset);
            value = static_cast<double>(temp);
        }else{
            value = 0; // not defined
        }

        m_data_buffer[m_index_buffer][i] = value;
    }
    m_mutex->unlock();
    emit newData2(m_index_buffer);

    m_index_buffer++;

}

void Plots::deletePlot(int index){
    qDebug() << "Index: " << index;
    bool returnvalue;
    //m_layout->removeWidget(m_plots.at(index));
    // disconnecten!
    QLayoutItem* item;
    item = m_layout->takeAt( index );
    qDebug() << "Item -> widget(): " <<item->widget();
    if(item->widget() == m_plots.at(index)){
        returnvalue = disconnect(this, &Plots::newData2, m_plots.at(index), &Plot::newData);
        returnvalue = disconnect(m_signals, &Signals::signalsChanged, m_plots.at(index), &Plot::signalsChanged);
        disconnect(this, &Plots::resizePlotBuffer, m_plots.at(index), &Plot::resizePlotBuffer);
        qDebug() << "Deleting Object";
        item->widget()->deleteLater();
        m_plots.remove(index);
    }
}

void Plots::exportSettings(){
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Export Settings"), "/home",
            tr("UDP Logger Config Files (*.udpLoggerSettings)"));


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

    QJsonArray active_signals;
    for (int i=0; i< m_signals->getSignalCount();i++){
        QJsonObject signal;
        signal["Datatype"] = m_signals->getSignal(i).datatype;
        signal["Index"] = m_signals->getSignal(i).index;
        signal["Offset"]  = m_signals->getSignal(i).offset;
        signal["Signalname"]  = m_signals->getSignal(i).name;
        active_signals.append(signal);
    }
    object["Signals"] = active_signals;



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

    // before importing remove everything!!!!
    int size = m_plots.size();
    for (int i=0; i<size; i++){
        deletePlot(0);
    }

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open settingsfile"), "/home", tr("UDP Logger Config Files (*.udpLoggerSettings)"));

    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

    QJsonObject object = d.object();

    if(object.contains("Signals")){
        QJsonArray used_signals = object["Signals"].toArray();
        QVector<struct Signal> signal_vector;
        struct Signal signal_element;

        for(int i=0; i<used_signals.count(); i++){
            QJsonObject signal = used_signals[i].toObject();
            signal_element.datatype = signal["Datatype"].toString();
            signal_element.index = signal["Index"].toInt();
            signal_element.offset = signal["Offset"].toInt();
            signal_element.name = signal["Signalname"].toString();
            signal_vector.append(signal_element);
        }
        m_signals->setSignals(&signal_vector);
    }

    if(object.contains("Plots")){
        QJsonArray plots = object["Plots"].toArray();

        for(int i=0; i<plots.count(); i++){
            createNewPlot();

            QJsonObject plot = plots[i].toObject();
            if(plot.contains("Graphs")){
                QJsonArray graphs = plot["Graphs"].toArray();
                for(int j=0; j<graphs.count(); j++){

                    QJsonObject graph = graphs[j].toObject();
                    if (graph.contains("Color")&&graph.contains("GraphName")&&
                            graph.contains("LineStyle") && graph.contains("ScatterStyle")&&
                            graph.contains("Signal")){
                        struct SettingsGraph settings;
                        settings.name = graph["GraphName"].toString();
                        settings.color = QColor(graph["Color"].toString());
                        settings.scatterstyle = graph["ScatterStyle"].toInt();
                        settings.linestyle = graph["LineStyle"].toInt();

                        QJsonObject signal_settings = graph["Signal"].toObject();

                        struct Signal signal;
                        signal.datatype = signal_settings["Datatype"].toString();
                        signal.index = signal_settings["Index"].toInt();
                        signal.offset = signal_settings["Offset"].toInt();
                        signal.name = signal_settings["Signalname"].toString();
                        settings.signal = signal;

                        m_plots[i]->addGraphToPlot(&settings);
                    }else{
                        qDebug() << "Some settings not found";
                    }



                }
            }
        }
    }

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
    Plot* plot = new Plot(this, m_parent, m_plot_buffersize,m_udp_buffersize, m_plots.length(),m_signals);
    connect(this, &Plots::resizePlotBuffer, plot, &Plot::resizePlotBuffer);
    m_plots.append(plot);
    m_layout->addWidget(plot);
    connect(this, &Plots::newData2, plot, &Plot::newData);
    connect(m_signals, &Signals::signalsChanged, plot, &Plot::signalsChanged);
    return plot;
}

void Plots::startUDP(){
    //m_udp->connectDataReady();
    emit connectToReadyRead();
    m_ifudpLogging = 1;
}

void Plots::stopUDP(){
    //m_udp->disconnectDataReady();
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

void Plots::settingsAccepted(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int data_buffersize, int port, bool export_data, QString export_filename){
    m_project_name = project_name;
    m_hostaddress = hostname;
    m_plot_buffersize = plot_buffersize;
    m_udp_buffersize = udp_buffersize;
    m_data_buffersize = data_buffersize;
    m_port = port;
    m_export_data = export_data;
    changeDataBufferSize(data_buffersize, udp_buffersize);
    emit initUDP(hostname,static_cast<quint16>(port),udp_buffersize, export_data, export_filename);

    emit resizePlotBuffer(m_udp_buffersize, m_plot_buffersize);
}

void Plots::changeDataBufferSize(int data_buffersize, int udp_buffersize){
    m_data_buffersize=data_buffersize;
    m_udp_buffersize = udp_buffersize;
    m_data_buffer.resize(data_buffersize);

    for(int i=0; i<data_buffersize; i++){
        m_data_buffer[i].resize(udp_buffersize);
    }
}

Plots::~Plots(){
    m_udp_thread.quit();
    m_udp_thread.wait(); // waiting till thread quits
    delete m_udp;
    delete m_mutex;
}
