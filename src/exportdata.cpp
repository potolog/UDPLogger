#include "exportdata.h"
#include "signals.h"
#include "plotbuffer.h"
#include "datafile.h"

ExportData::ExportData(QString path, QVector<struct udp_message_puffer> &data,int index_first, int index_last,int  udp_buffer_size, Signals* signal, QObject *parent) :
    QThread(parent), m_path(path), m_first(index_first),m_last(index_last), m_udp_buffer_size(udp_buffer_size), m_signals(signal)
{

    int start_index = 0;
    if(m_first < 0){
        start_index = m_udp_buffer_size-m_first;
    }else{
        start_index = m_first;
    }

    m_data.resize(m_last-m_first);
    int index = 0;
    for(int i=start_index; i< (m_last - m_first); i++){
        if(i>= m_udp_buffer_size){
            i=0;// circle buffer
        }
        m_data[index] = data[i];
        index ++;
    }
}

void ExportData::run(){
    QString result;
    QVector<QVector<double>> data;
    for(int row = 0; row < m_data.length(); row++){
        for(int column= 0; column< m_signals->getSignalCount(); column++){
            data[row][column] = PlotBuffers::getValue(m_data[row].puffer,UDP_CONSTANTS::max_data,m_signals->getSignal(column));
        }
    }
    QVector<QString> names;
    QVector<QString> units;
    names.resize(m_signals->getSignalCount());
    units.resize(m_signals->getSignalCount());

    for(int i=0; i<m_signals->getSignalCount(); i++){
        struct Signal signal = m_signals->getSignal(i);
        names[i] = signal.name;
        units[i] = signal.unit;
    }

    DataFile datafile(m_path);
    datafile.writeDataFile(names,units,data);
    Q_EMIT resultReady(result);
}
