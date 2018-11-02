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

#include "udp.h"
#include <QUdpSocket>
#include <iostream>
#include <array>
#include <QtDebug>
#include <QMutex>
#include <QFile>
#include <QTimer>
#include <sys/time.h>

#include "plots.h"
#include <qcustomplot.h>
#include "plotbuffer.h"
#include "signals.h"
#include "triggerwidget.h"
#include "datafile.h"

#include "exportdata.h"


#include <QMessageBox>

UDP::UDP(Plots *parent, QMutex *mutex, PlotBuffers *data_buffers, Signals *signal, TriggerWidget *trigger):
    m_mutex(mutex), m_parent(parent), m_data_buffers(data_buffers), m_signals(signal),m_triggerwidget(trigger)
{
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &UDP::readData);
    m_ifread_data = 0;

    m_timer = new QTimer(this);

    connect(this, &UDP::triggerFinished, trigger, &TriggerWidget::triggered);
    connect(m_timer, &QTimer::timeout,this,&UDP::timerTimeout);

    m_triggered = false;

}
bool UDP::init(){
    return init(QHostAddress::AnyIPv4, 60000, 400,200,10, false,1, "");
}

bool UDP::init(QHostAddress hostaddress, quint16 port, int udp_buffer_size,int data_buffer_size,int redraw_count, bool export_data, int use_data_count, QString filename){

    m_socket->disconnectFromHost();
    m_use_data_count = use_data_count;
    m_if_file_ready = 0;
    m_data_buffer_size = data_buffer_size;
    m_redraw_count = redraw_count;
    m_udp_buffer_size = udp_buffer_size;
    m_udp_buffer.resize(udp_buffer_size);
    m_udp_index = 0;
    m_filename = filename;

    m_buffer_smaller_than_message = 0;

    m_actual_index = 0;

    if (!m_socket->bind(hostaddress, port)){
        QMessageBox msgBox;

        msgBox.setText("Hostaddress or Port not valid: "+m_socket->errorString());
        msgBox.exec();
        std::cout << "Bind: NOK" << std::endl;
        return 0;
    }

    std::cout << "Bind: OK" << std::endl;

    m_export_data = export_data;

    return 1;
}

void UDP::readData(){

    //uint64_t difference = calculateTimedifference();

    if(m_actual_index >= m_use_data_count){
        m_actual_index = 0;
    }

    if(m_udp_index >= m_udp_buffer_size){
        m_udp_index = 0;
    }

    struct udp_message_puffer puffer;
    qint64 size =  m_socket->readDatagram(puffer.puffer,UDP_CONSTANTS::max_data,nullptr, nullptr);

    if(size > UDP_CONSTANTS::max_data && m_buffer_smaller_than_message ==0){
        m_buffer_smaller_than_message = 1;
        QMessageBox::information(nullptr,tr("UDP message size greater than max data"),tr("The received UDP message size (")+QString::number(size)+tr(" Byte) is greater than the maximum allowed message size of ")+QString::number(UDP_CONSTANTS::max_data)+" Byte."+
                                 tr("This means, that not every signal can be plotted"));
    }

    m_mutex->lock(); // brauchts das locken überhaupt?
    m_udp_buffer[m_udp_index] = puffer;
    if(m_ifread_data && m_actual_index==0){ // only every m_m_use_data_count data should be plottet
        // Attention, m_m_use_data_count and m_redrawcount problem
        // Add data to data buffer, so the diagrams will be refreshed
        m_data_buffers->addData(puffer.puffer,UDP_CONSTANTS::max_data);
    }
    m_mutex->unlock();

    if(m_triggerwidget->isTriggerEnabled()){
        double value = m_data_buffers->getValue(puffer.puffer, UDP_CONSTANTS::max_data,m_triggerwidget->getTriggerSignal());
        double trigger_level = m_triggerwidget->getTriggerLevel();
        TriggerType triggertype = m_triggerwidget->getTriggerType();
        if(value > trigger_level && value > m_previous_value && (triggertype == TriggerType::RISING_EDGE || triggertype == TriggerType::ALL_EDGES) ){
            m_triggered = true;
            m_timer->start(m_triggerwidget->getTimeAfterTrigger()*1000);
        }else if(value < trigger_level && value < m_previous_value && (triggertype == TriggerType::FALLING_EDGE || triggertype == TriggerType::ALL_EDGES)){
            m_triggered = true;
            m_timer->start(m_triggerwidget->getTimeAfterTrigger()*1000);
        }

        if(m_triggered){
            m_triggerwidget->disableTrigger();
            m_previous_value = value;
            m_trigger_index = m_udp_index;
        }
    }

    m_actual_index ++;
    m_udp_index++;
}

void UDP::connectDataReady(){
    m_ifread_data = 1;
}

void UDP::disconnectDataReady(){
    m_ifread_data = 0;
}

int64_t UDP::calculateTimedifference(){
    struct timeval time;
    gettimeofday(&time,nullptr);
    int64_t actual_time = time.tv_sec+time.tv_usec*1000000;
    int64_t difference = actual_time - m_time_state;

    return difference;
}

struct udp_message_puffer UDP::getValueIndexBefore(int index){
    int index_new = m_udp_index - index;
    if(index_new < 0){
      index_new = m_udp_buffer_size-index_new;
    }

    return m_udp_buffer[index_new];
}

void UDP::timerTimeout(){
    double time_after = m_triggerwidget->getTimeAfterTrigger();
    double time_before = m_triggerwidget->getTimeBeforeTrigger();

    int index_before = m_trigger_index - static_cast<int>(time_before/time_after*(m_udp_index-m_trigger_index));

    if(m_udp_buffer_size -(m_udp_index-index_before)< 0){
        QMessageBox::information(m_parent,tr("Bufferoverflow of UDP buffer"),tr("With the actual trigger time settings, the number of values which should be stored (")+QString::number(m_udp_index-index_before)+") "+tr("is higher than the UDP buffer size. So the data repeats. Please set the trigger times lower or set the UDP buffersize to a higher value"));
    }

    m_export = new ExportData(m_filename,m_udp_buffer,index_before, m_udp_index, m_udp_buffer_size,m_signals,this);
    connect(m_export, &ExportData::resultReady, this, &UDP::exportFinished);
    m_export->run();
}

void UDP::exportFinished(){
    m_export->deleteLater();
    Q_EMIT triggerFinished();
}

UDP::~UDP(){
    delete m_socket;
}
