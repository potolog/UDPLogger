#include "exportdata.h"
#include "signals.h"
#include "plotbuffer.h"
#include "datafile.h"
#include <netcdf>
#include <vector>

ExportData::ExportData(QString path,QString project_name, QVector<struct udp_message_puffer> &data,int index_first, uint64_t index_last,int  udp_buffer_size, Signals* signal, QObject *parent) :
    QThread(parent), m_path(path), m_first(index_first),m_last(index_last), m_udp_buffer_size(udp_buffer_size), m_signals(signal), m_project_name(project_name)
{

    int start_index = 0;
    if(m_first < 0){
        start_index = m_udp_buffer_size-m_first;
    }else{
        start_index = m_first;
    }

    m_data.resize(m_last-m_first);
    int index = 0;
    int buffer_index = start_index;
    for(int i=0; i< (m_last - m_first); i++){ // m_first can be negative, so subtraction is possible
        if(buffer_index >= m_udp_buffer_size){
			buffer_index= buffer_index % m_udp_buffer_size;
        }
		m_data[index] = data[buffer_index];
		buffer_index++;
        index ++;
    }
}

void ExportData::run(){

    using namespace netCDF;
    using namespace netCDF::exceptions;

    double *data = new double[m_data.length()*m_signals->getSignalCount()];
    for(int row = 0; row < m_signals->getSignalCount(); row++){
        for(int column= 0; column< m_data.length(); column++){
            double value = PlotBuffers::getValue(m_data[column].puffer,UDP_CONSTANTS::max_data,m_signals->getSignal(row));
            data[row*m_data.length()+column] = value;
        }
    }
    try {

        if(m_path.isEmpty()){
            emit showInfoMessage(tr("No output path!"),tr("Please define export path in the Settings to use the trigger!"));
            emit resultReady();
            return;
        }
        QString date = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        std::string output_file_name = (m_path+"/"+m_project_name+"_"+date+".hdf5").toStdString();

        NcFile::FileFormat format = NcFile::nc4;
        NcFile datafile(output_file_name,NcFile::replace,format);

        for(int i=0; i<m_signals->getSignalCount(); i++){
            struct Signal signal = m_signals->getSignal(i);
            NcDim dim = datafile.addDim(signal.name.toStdString(), m_data.length());
            NcVar dataVar = datafile.addVar(signal.name.toStdString(), ncDouble, dim);
            dataVar.putAtt("units",signal.unit.toStdString());
            dataVar.putVar(&data[i*m_data.length()]);
        }

    } catch (NcException& e) {
        e.what();
    }
    delete[] data;
    emit resultReady();
}
