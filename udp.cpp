#include "udp.h"
#include <QUdpSocket>
#include <iostream>
#include <array>
#include <QtDebug>
#include <QMutex>
#include "plots.h"

UDP::UDP(Plots *parent, QMutex *mutex): m_mutex(mutex), m_parent(parent)
{
    m_socket = new QUdpSocket(this);
    connect(this, &UDP::newData, m_parent, &Plots::newData);
}
bool UDP::init(){
    return init(QHostAddress::Any, 60000, 400);
}

bool UDP::init(QHostAddress hostaddress, quint16 port, int buffer_size){
    m_data.resize(buffer_size);
    m_actual_index = 0;

    if (m_socket->bind(hostaddress, port)){
        std::cout << "Bind: OK" << std::endl;
        return 0;
    }else{
        QMessageBox msgBox;
        msgBox.setText("Hostaddress or Port not valid");
        msgBox.exec();
        std::cout << "Bind: NOK" << std::endl;
        return -1;
    }
}

void UDP::readData(){

    if(m_actual_index >= m_data.size()){
        m_actual_index = 0;
    }

    m_mutex->lock();
    qint64 size =  m_socket->readDatagram(m_data.data(),m_data.size(),nullptr, nullptr);
    m_mutex->unlock();

    emit newData();

    if (size != 128){
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
