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
#include "plotbuffer.h"

class UDP;
class QMutex;
class QJsonObject;
class SettingsDialog;
class Plot;
class TriggerWidget;


class Plots: public QWidget
{
    Q_OBJECT

public:
    Plots(QWidget *parent, Signals* signal, TriggerWidget *trigger);
    QVBoxLayout* getLayout(){return m_layout;}
    Plot* createNewPlot();
    inline double* getBufferData(){return m_data_buffer.data()->data();}
    inline double getBufferData(int row, int column){return m_data_buffer[row][column];}
    inline unsigned long getbufferIndex(){return m_index_buffer;}
    inline int getBufferCount(){return m_udp_buffersize;}
    void changeDataBufferSize(int data_buffersize, int udp_buffersize);
    inline int getRedrawCounter(){return m_redraw_count;}
    inline int getDataBufferSize(){return m_data_buffersize;}
    inline int getSkipElement(){return m_use_data_count;}
    ~Plots();
    void removeGraph(struct Signal xaxis, struct Signal yaxis);
    QSharedPointer<QCPGraphDataContainer> getBuffer(struct Signal xaxis, struct Signal yaxis);
    QWidget* getParent(){return m_parent;}
public slots:
    void deletePlot(int index);
    void exportSettings();
    void importSettings();
    void settings();
    void startUDP();
    void stopUDP();
    void settingsAccepted(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int data_buffersize, int port, int redraw_count, int use_data_count, QString export_path, QString relative_header_path);
    void showInfoMessageBox(QString title, QString text);
signals:
    void startUDPReadData();
    void resizePlotBuffer(int udp_buffersize, int plot_buffersize);
    void connectToReadyRead();
    void disconnectToReadyRead();
    void initUDP(QHostAddress hostaddress, quint16 port, int buffer_size,int data_size, int redraw_count, int m_use_data_count, QString export_path, QString project_name);
    void changeRelativeHeaderPath(QString relative_header_path);
    void dataBufferSizeChanged(int data_buffer_size);
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
    QString m_export_filename;
    int m_redraw_count;
    unsigned long m_index_new_data; // used with m_redraw_count
    int m_use_data_count;

    QString m_program_version = "2.0";
    QString m_project_name;
    bool m_ifudpLogging;
    SettingsDialog* m_settings_dialog;


    unsigned long m_index_buffer;
    int m_data_buffersize;
    QVector<QVector<double>> m_data_buffer;

    PlotBuffers *m_data_buffers;

};

#endif // PLOTS_H
