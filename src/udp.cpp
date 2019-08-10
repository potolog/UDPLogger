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

UDP::UDP(Plots *parent, QMutex *mutex, PlotBuffers *data_buffers, Signals *signal, TriggerWidget *trigger):
    m_mutex(mutex), m_parent(parent), m_data_buffers(data_buffers), m_signals(signal),m_triggerwidget(trigger)
{
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::readyRead, this, &UDP::readData);
    m_ifread_data = 0;

    m_trigger_in_progress = false;

    m_timer = new QTimer(this);
	m_actual_value = 0;


    connect(this, &UDP::triggerFinished, trigger, &TriggerWidget::triggered, Qt::ConnectionType::QueuedConnection);
    connect(this, &UDP::disableTrigger, trigger, &TriggerWidget::disableTrigger, Qt::ConnectionType::BlockingQueuedConnection);
	connect(this, &UDP::newTriggerValue, trigger, &TriggerWidget::newTriggerValue, Qt::ConnectionType::QueuedConnection);
	connect(trigger, &TriggerWidget::startTrigger, this, &UDP::startTrigger, Qt::ConnectionType::BlockingQueuedConnection);
	connect(this, &UDP::triggerStarted, trigger, &TriggerWidget::triggerStarted, Qt::ConnectionType::QueuedConnection);
	connect(m_timer, &QTimer::timeout, this, &UDP::refreshPlot);




}
bool UDP::init(){
    return init(QHostAddress::AnyIPv4, 60000,200,10,1, "", "");
}

bool UDP::init(QHostAddress hostaddress, quint16 port, int udp_buffer_size, int refresh_rate, int use_data_count, QString export_path, QString project_name){

    m_socket->disconnectFromHost();
    m_use_data_count = use_data_count;
    m_if_file_ready = 0;
    m_refresh_rate = refresh_rate;
    m_udp_buffer_size = udp_buffer_size;
    m_udp_buffer.resize(udp_buffer_size);
    m_udp_index = 0;
    m_filename = export_path;
    m_project_name = project_name;
    m_data_changed = false;
    m_trigger_in_progress = false;

    m_buffer_smaller_than_message = 0;

    m_actual_index = 0;
    m_udp_global_index = 0;

    if (!m_socket->bind(hostaddress, port)){
		emit showInfoMessageBox(tr("Not able to open UDP"),QString(tr("Hostaddress or Port not valid: '%1'\nMaybe a second instance of UDPLogger is open?")).arg(m_socket->errorString()));
        std::cout << "Bind: NOK" << std::endl;
        return 0;
    }

    m_timer->start(1000/refresh_rate);

    std::cout << "Bind: OK" << std::endl;

    return 1;
}

void UDP::refreshPlot(){
    if(m_data_changed){
        m_data_changed = false;
        emit dataChanged();
		emit newTriggerValue(m_actual_value);
    }
}

void UDP::readData(){

    m_time_difference = calculateTimedifference();

    if(m_actual_index >= m_use_data_count){
        m_actual_index = 0;
    }

    if(m_udp_index >= m_udp_buffer_size){
        m_udp_index = 0;
    }
    struct udp_message_puffer puffer;
    qint64 size =  m_socket->readDatagram(puffer.puffer,UDP_CONSTANTS::max_data,nullptr, nullptr);

    if(size > UDP_CONSTANTS::max_data && m_buffer_smaller_than_message ==0){ // only one time
        m_buffer_smaller_than_message = 1;
        showInfoMessageBox(tr("UDP message size greater than max data"),tr("The received UDP message size (")+QString::number(size)+tr(" Byte) is greater than the maximum allowed message size of ")+QString::number(UDP_CONSTANTS::max_data)+" Byte."+
                           tr("This means, that not every signal can be plotted"));
    }

    // save data in udp buffer
    m_mutex->lock(); // brauchts das locken überhaupt?
    m_udp_buffer[m_udp_index] = puffer;

    // store data in data buffer which is the buffer for the plot
    if(m_ifread_data && m_actual_index==0){ // only every m_m_use_data_count data should be plottet
        // Attention, m_m_use_data_count and m_redrawcount problem
        // Add data to data buffer, so the diagrams will be refreshed
        m_data_buffers->addData(puffer.puffer,UDP_CONSTANTS::max_data);
        m_data_changed = true;
    }

    m_mutex->unlock();

	if(m_signals->getSignalCount() > 0){
		m_actual_value = m_data_buffers->getValue(puffer.puffer, UDP_CONSTANTS::max_data,m_triggerwidget->getTriggerSignal());
	}
    // Trigger
    if(m_triggerwidget->isTriggerEnabled() && !m_trigger_in_progress){
		double trigger_level = m_triggerwidget->getTriggerLevel();
        TriggerType triggertype = m_triggerwidget->getTriggerType();
        bool triggered = false;
		if(m_actual_value > trigger_level && m_actual_value > m_previous_value && (triggertype == TriggerType::RISING_EDGE || triggertype == TriggerType::ALL_EDGES) ){
            triggered = true;
		}else if(m_actual_value < trigger_level && m_actual_value < m_previous_value && (triggertype == TriggerType::FALLING_EDGE || triggertype == TriggerType::ALL_EDGES)){
            triggered = true;
		}else{
			m_previous_value = m_actual_value;
		}

        if(triggered){
			startTrigger();
        }
    }

    m_actual_index ++;
    m_udp_index++;
    m_udp_global_index++;
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

    emit triggerFinished();

    double time_after = m_triggerwidget->getTimeAfterTrigger();
    double time_before = m_triggerwidget->getTimeBeforeTrigger();

    int index_before;
    if(std::abs(time_after) <= 0.000001){
        if(std::abs(time_before) <= 0.000001){
            emit disableTrigger();
            emit showInfoMessageBox(QObject::tr("Trigger times invalid"), QObject::tr("Please set 'Time before trigger' or 'Time after trigger' unequal to zero"));
			exportFinished();
            return;
        }
        index_before = m_trigger_index - static_cast<int>(time_before)/m_time_difference;
    }else{
		// can be assumed, that index_before don't get an overflow because it is a 64bit variable
        index_before = m_trigger_index - static_cast<int>(time_before/time_after*(m_udp_global_index-m_trigger_index));
    }


     // index_before can also be negative, so it is possible to just subtract
    if(m_udp_buffer_size -(m_udp_global_index-index_before)< 0){
        emit showInfoMessageBox(QObject::tr("Bufferoverflow of UDP buffer"),
                                QObject::tr("With the actual trigger time settings, "
                                            "the number of values which should be stored (")+
                                QString::number(m_udp_global_index-index_before)+") "+tr("is higher "
                                "than the UDP buffer size (")+QString::number(m_udp_buffer_size)+tr("). So the data repeats. Please set "
                                "the trigger times lower or set the UDP buffersize to a higher value"));

		exportFinished();
        return;
    }

    m_export = new ExportData(m_filename,m_project_name,m_udp_buffer,index_before, m_udp_global_index, m_udp_buffer_size,m_signals,this);
    connect(m_export, &ExportData::resultReady, this, &UDP::exportFinished);
	connect(m_export, &ExportData::resultReady, m_triggerwidget, &TriggerWidget::triggerFinished);
    connect(m_export, &ExportData::showInfoMessage, m_parent,&Plots::showInfoMessageBox, Qt::ConnectionType::QueuedConnection);
    m_export->run();
}

void UDP::exportFinished(){
	emit triggerFinished();
	if (m_export) {
		m_export->deleteLater();
		m_export = nullptr;
	}
    m_trigger_in_progress = false;
}

void UDP::startTrigger(){
	emit triggerStarted();
	QTimer::singleShot(m_triggerwidget->getTimeAfterTrigger()*1000,this,&UDP::timerTimeout);
	m_trigger_in_progress = true;
	m_previous_value = m_data_buffers->getValue(m_udp_buffer[m_udp_index].puffer, UDP_CONSTANTS::max_data,m_triggerwidget->getTriggerSignal());
	m_trigger_index = m_udp_global_index;
}

UDP::~UDP(){
    delete m_socket;
}
