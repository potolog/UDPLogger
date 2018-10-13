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

#ifndef PLOTS_H
#define PLOTS_H

#include "signals.h"
#include <array>
#include <QHostAddress>
#include <QtWidgets>

class UDP;
class QMutex;
class QJsonObject;
class SettingsDialog;
class Plot;


class Plots: public QWidget
{
    Q_OBJECT

public:
    Plots(QWidget *parent, Signals* signal);
    QVBoxLayout* getLayout(){return m_layout;}
    Plot* createNewPlot();
    double* getBufferData(){return m_data_buffer.data()->data();}
    double getBufferData(int row, int column){return m_data_buffer[row][column];}
    unsigned long getbufferIndex(){return m_index_buffer;}
    int getBufferCount(){return m_udp_buffersize;}
    void changeDataBufferSize(int data_buffersize, int udp_buffersize);
    int getRedrawCounter(){return m_redraw_count;}
    int getDataBufferSize(){return m_data_buffersize;}
    int getSkipElement(){return m_use_data_count;}
    ~Plots();
public slots:
    void deletePlot(int index);
    void newData();
    void exportSettings();
    void importSettings();
    void settings();
    void startUDP();
    void stopUDP();
    void settingsAccepted(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int data_buffersize, int port, bool export_data, int redraw_count, int use_data_count, QString export_filename, QString relative_header_path);
signals:
    void startUDPReadData();
    void newData2(unsigned long m_index_buffer);
    void resizePlotBuffer(int udp_buffersize, int plot_buffersize);
    void connectToReadyRead();
    void disconnectToReadyRead();
    void initUDP(QHostAddress hostaddress, quint16 port, int buffer_size, bool export_data,int m_use_data_count, QString filename);
    void changeRelativeHeaderPath(QString relative_header_path);
private:
    QWidget* m_parent;
    QVector<Plot*> m_plots;
    QVBoxLayout* m_layout;
    Signals* m_signals;
    QThread m_udp_thread;
    UDP* m_udp;
    QMutex* m_mutex;
    QHostAddress m_hostaddress;
    int m_port;
    int m_udp_buffersize;
    int m_plot_buffersize;
    bool m_export_data;
    QString m_export_filename;
    int m_redraw_count;
    unsigned long m_index_new_data; // used with m_redraw_count
    int m_use_data_count;

    QString m_program_version;
    QString m_project_name;
    bool m_ifudpLogging;
    SettingsDialog* m_settings_dialog;


    unsigned long m_index_buffer;
    int m_data_buffersize;
    QVector<QVector<double>> m_data_buffer;

};

#endif // PLOTS_H
