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
    inline int getRedrawCounter(){return m_refresh_rate;}
    inline int getSkipElement(){return m_use_data_count;}
    ~Plots();
    void removeGraph(struct Signal xaxis, struct Signal yaxis);
    QSharedPointer<QCPGraphDataContainer> getBuffer(struct Signal xaxis, struct Signal yaxis);
    QWidget* getParent(){return m_parent;}
public slots:
    void deletePlot(Plot *plot_address);
	void saveSettings();
	void saveSettingsAs();
	void exportSettings(QString fileName);
    void importSettings();
    void settings();
    void startUDP();
    void stopUDP();
	void settingsAccepted(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int port, int refresh_rate, int use_data_count, QString export_path, QString sourcePath, QString relative_header_path, QString additional_includes);
    void showInfoMessageBox(QString title, QString text);
    void clearPlots();
signals:
    void startUDPReadData();
    void connectToReadyRead();
    void disconnectToReadyRead();
    void initUDP(QHostAddress hostaddress, quint16 port, int udp_buffer_size, int refresh_rate, int m_use_data_count, QString export_path, QString project_name);
	void changeSignalSettings(QString sourcePath, QString relative_header_path, QString additional_includes);
    void plotBufferSizeChanged(int data_buffer_size);
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
    QString m_export_filename;
    int m_refresh_rate;
    unsigned long m_index_new_data; // used with m_refresh_rate
    int m_use_data_count;

	const QString m_program_version = "2.5";
    QString m_project_name;
    bool m_ifudpLogging;
    SettingsDialog* m_settings_dialog;

    PlotBuffers *m_data_buffers;

};

#endif // PLOTS_H
