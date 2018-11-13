#include "plotbuffer.h"


PlotBuffer::PlotBuffer(){

}

PlotBuffer::PlotBuffer(int index_x, int index_y): m_index_x(index_x), m_index_y(index_y){
    m_count = 0;
}

int PlotBuffer::decreaseCounter() {
    m_count--;
    return m_count;
}

PlotBuffers::PlotBuffers(Signals* signal):QObject(nullptr), m_signals(signal){
}

PlotBuffers::~PlotBuffers(){}

void PlotBuffers::plotBufferSizeChanged(int plot_buffer_size){
    m_plot_buffer_size = plot_buffer_size;
}

QSharedPointer<QCPGraphDataContainer> PlotBuffers::getBuffer(struct Signal xaxis, struct Signal yaxis){

    for(int i=0; i<m_plot_buffer.length(); i++){
        auto index_x = m_plot_buffer[i]->getIndexX();
        auto index_y = m_plot_buffer[i]->getIndexY();


        if(index_x == xaxis.index && index_y == yaxis.index){
            m_plot_buffer[i]->increaseCounter();
            return m_plot_buffer[i];
        }
    }
    QSharedPointer<PlotBuffer> buffer = QSharedPointer<PlotBuffer>(new PlotBuffer(xaxis.index, yaxis.index));
    m_plot_buffer.append(buffer);
    m_plot_buffer[m_plot_buffer.length()-1]->increaseCounter();
    return m_plot_buffer[m_plot_buffer.length()-1];
}

// decreases the counter of the buffer which holds pointer_old
void PlotBuffers::removeSignal(struct Signal xaxis, struct Signal yaxis){
    for(int i=0; i<m_plot_buffer.length(); i++){
        int index_x = m_plot_buffer[i]->getIndexX();
        int index_y = m_plot_buffer[i]->getIndexY();
        if(index_x == xaxis.index && index_y == yaxis.index){
            if(m_plot_buffer[i]->decreaseCounter()<=0){
                m_plot_buffer.removeAt(i);
            }
            return;
        }
    }
}

void PlotBuffers::removeGraph(struct Signal xaxis, struct Signal yaxis){
    for (int i=0; i<m_plot_buffer.length(); i++){
        auto index_x = m_plot_buffer[i]->getIndexX();
        auto index_y = m_plot_buffer[i]->getIndexY();

        if(index_x == xaxis.index && index_y == yaxis.index){
            int counter = m_plot_buffer[i]->decreaseCounter();
            if(counter <=0){ // should never be that the counter decreases below 0
                m_plot_buffer.removeAt(i);
            }
        }
    }
}

double PlotBuffers::getValue(char* data, int length, struct Signal signal){

    if(signal.datatype.compare("float")== 0){
        if(length > signal.offset+4-1){ // preventing pufferoverflow
            float temp =*getFloatPointer(signal.offset,data);
            return static_cast<double>(temp);
        }
    }else if(signal.datatype.compare("double")==0){
        if(length > signal.offset+8-1){
            return *getDoublePointer(signal.offset,data);
        }

    }else if(signal.datatype.compare("int8_t")==0){
        if(length > signal.offset){
            char temp = *getCharPointer(signal.offset,data);
            return static_cast<double>(temp);
        }

    }else if(signal.datatype.compare("uint8_t")==0){
        if(length > signal.offset){
            uint8_t temp = *getUCharPointer(signal.offset,data);
            return static_cast<double>(temp);
        }

    }else if(signal.datatype.compare("int16_t")==0){
        if(length > signal.offset+2-1){
            int16_t temp = *getShortPointer(signal.offset,data);
            return static_cast<double>(temp);
        }

    }else if(signal.datatype.compare("uint16_t")==0){
        if(length > signal.offset+2-1){
            uint16_t temp = *getUShortPointer(signal.offset,data);
            return static_cast<double>(temp);
        }

    }else if(signal.datatype.compare("bool")==0){
        if(length > signal.offset){
            bool temp = *getBoolPointer(signal.offset,data);
            return static_cast<double>(temp);
        }

    }else if(signal.datatype.compare("int32_t")==0){
        if(length > signal.offset+4-1){
            int temp = *getIntPointer(signal.offset,data);
            return static_cast<double>(temp);
        }
    }else if(signal.datatype.compare("uint32_t")==0){
        if(length > signal.offset+4-1){
            int temp = *getUIntPointer(signal.offset,data);
            return static_cast<double>(temp);
        }
    }else{
        return 0; // not defined
    }
}

void PlotBuffers::addData(char* data, int length){
    for(int i=0; i<m_plot_buffer.length(); i++){
        int index_xaxis = m_plot_buffer[i]->getIndexX();
        int index_yaxis = m_plot_buffer[i]->getIndexY();

        const int signal_count = 2;

        struct Signal signal_axis[signal_count]; // 0: xaxis, 1: yaxis
        signal_axis[0] = m_signals->getSignal(index_xaxis);
        signal_axis[1] = m_signals->getSignal(index_yaxis);

        double value[signal_count] = {0}; // init to zero

        for(int j=0; j<signal_count; j++){
            value[j] = getValue(data,length,signal_axis[j]);
        }

        //QCPGraphDataContainer data;
        //data.add(new_data);
        //auto value = data.at(0);
        //data.remove(value->key);
        //m_plot_buffer = new QVector<PlotBuffer>;
        QVector<QCPGraphData> new_data = {QCPGraphData(value[0],value[1])};
        m_plot_buffer[i]->add(new_data,true);

        for(int j=0; j<m_plot_buffer[i]->size()-m_plot_buffer_size; j++){
            auto value_at_index = m_plot_buffer[i]->at(0);
            m_plot_buffer[i]->remove(value_at_index->key);
        }
    }
}

void PlotBuffers::clearPlots(){
    for(int i=0; i< m_plot_buffer.length(); i++){
        m_plot_buffer[i].data()->clear();
    }
}
