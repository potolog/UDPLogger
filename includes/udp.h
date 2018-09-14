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

class UDP: public QObject
{
    Q_OBJECT

public:
    UDP(Plots* parent, QMutex* mutex);
    bool init(QHostAddress hostaddress, quint16 port, int buffer_size, bool export_data, QString filename);
    bool init();
    ~UDP();
public slots:
    void readData();
    float* getFloatPointer(int position){return (float*)(&m_data.data()[position]);}
    double* getDoublePointer(int position){return (double*)(&m_data.data()[position]);}
    char* getCharPointer(int position){return &m_data.data()[position];}
    int* getIntPointer(int position){return (int*)(&m_data.data()[position]);}
    void connectDataReady();
    void disconnectDataReady();

signals:
    void newData();

private:
    int m_actual_index;
    QUdpSocket *m_socket;
    QVector<char> m_data;
    QVector<char> m_data_temp;
    const int m_max_data = 400;

    bool m_export_data;
    QFile* file;
    bool m_if_file_ready;

    QMutex* m_mutex;
    unsigned long m_index_read; // index which data was read
    Plots* m_parent;



};

#endif // UDP_H
