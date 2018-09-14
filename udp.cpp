#include "udp.h"
#include <QUdpSocket>
#include <iostream>
#include <array>
#include <QtDebug>
#include <QMutex>
#include "plots.h"
#include <QFile>

UDP::UDP(Plots *parent, QMutex *mutex): m_mutex(mutex), m_parent(parent)
{
    m_socket = new QUdpSocket(this);
    connect(this, &UDP::newData, m_parent, &Plots::newData);
}
bool UDP::init(){
    return init(QHostAddress::Any, 60000, 400, false, "");
}

bool UDP::init(QHostAddress hostaddress, quint16 port, int buffer_size, bool export_data, QString filename = ""){
    m_socket->close();
    m_socket = new QUdpSocket(this);

    m_data.resize(buffer_size);
    m_data_temp.resize(buffer_size);
    m_if_file_ready = 0;

    m_actual_index = 0;

    if (!m_socket->bind(hostaddress, port)){
        QMessageBox msgBox;
        msgBox.setText("Hostaddress or Port not valid");
        msgBox.exec();
        std::cout << "Bind: NOK" << std::endl;
        return -1;
    }

    std::cout << "Bind: OK" << std::endl;

    m_export_data = export_data;
    if(m_export_data){
        QFile file( filename );
        if ( file.open(QIODevice::WriteOnly) )
        {
            m_if_file_ready = 1;
            QString header = "Buffersize: " + QString::number(m_data.size());
            //file.write();
        }
    }

    return 0;

}

void UDP::readData(){

    if(m_actual_index >= m_data.size()){
        m_actual_index = 0;
    }


    qint64 size =  m_socket->readDatagram(m_data_temp.data(),m_data_temp.size(),nullptr, nullptr);
    m_mutex->lock();
    m_data = m_data_temp;
    m_mutex->unlock();
    emit newData();

    if(m_if_file_ready){
        file->write(m_data_temp.data(), m_data_temp.size());
    }

    if (size != m_data_temp.size()){
        // Problem
        qDebug() << "UDP Size not correct";
    }

    m_actual_index ++;
}

void UDP::connectDataReady(){
    connect(m_socket, &QUdpSocket::readyRead, this, &UDP::readData);
}

void UDP::disconnectDataReady(){
    disconnect(m_socket, &QUdpSocket::readyRead, this, &UDP::readData);
}

UDP::~UDP(){
    delete m_socket;
}
