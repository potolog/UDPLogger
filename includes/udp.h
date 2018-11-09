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

#ifndef UDP_H
#define UDP_H

#include <array>
#include <QObject>
#include <QVector>

class QFile;
class QUdpSocket;
class QMutex;
class Plots;
class QHostAddress;
class PlotBuffers;
class Signals;
class TriggerWidget;
class QTimer;
class QThread;
class ExportData;

namespace UDP_CONSTANTS{
    const int max_data = 400;
}
struct udp_message_puffer{
    char puffer[UDP_CONSTANTS::max_data];
};

class UDP: public QObject
{
    Q_OBJECT

public:
    UDP(Plots* parent, QMutex* mutex, PlotBuffers* data_buffers, Signals* signal, TriggerWidget* trigger);
    bool init(QHostAddress hostaddress, quint16 port, int udp_buffer_size, int refresh_rate, int use_data_count, QString export_path, QString project_name);
    bool init();
    struct udp_message_puffer getValueIndexBefore(int index); // returns value "index" before actual m_udp_index
    int64_t calculateTimedifference();
    ~UDP();
public slots:
    void readData();
    void connectDataReady();
    void disconnectDataReady();
    void timerTimeout();
    void exportFinished();
    void refreshPlot();

signals:
    void newData();
    void triggerFinished();
    void dataChanged();
    void showInfoMessageBox(QString title, QString text);
    void disableTrigger();

private:
    int m_actual_index;
    int64_t m_udp_global_index;

    QUdpSocket *m_socket;
    int m_use_data_count;
    int m_udp_buffer_size;
    bool m_if_file_ready;

    QMutex* m_mutex;
    QThread* m_writing_data_thread;

    unsigned long m_index_read; // index which data was read
    Plots* m_parent;

    bool m_ifread_data;
    int m_data_buffer_size;

    bool m_data_changed;
    int m_refresh_rate;

    PlotBuffers *m_data_buffers;
    Signals* m_signals;
    int m_buffer_smaller_than_message;
    TriggerWidget *m_triggerwidget;
    double m_previous_value;
    int64_t m_trigger_index;
    bool m_trigger_in_progress;

    QString m_filename;
    QString m_project_name;

    // ring buffer
    int m_udp_index;
    QVector<struct udp_message_puffer> m_udp_buffer;

    int64_t m_time_state;

    QTimer* m_timer;

    ExportData* m_export;
    int64_t m_time_difference;


};

#endif // UDP_H
