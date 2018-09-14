#include "plot.h"
#include "plots.h"


#include <algorithm>
#include "plotscontextmenu.h"
#include "changegraphdialog.h"

Plot::Plot(Plots* plots, QWidget* parent, int plot_buffersize,int udp_buffersize, int index, Signals *signal):
    QCustomPlot(parent), m_index(index),m_parent(plots)
{

    connect(this, &Plot::deletePlot2, plots, &Plots::deletePlot);

    resizePlotBuffer(udp_buffersize, plot_buffersize);
    m_context_menu = new PlotsContextMenu();
    m_context_menu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_context_menu, &PlotsContextMenu::customContextMenuRequested,this,&Plot::ShowContextMenu);

    // visible context menu
    m_menu = new QMenu;

    QAction* change_graphstyle =new QAction(tr("Change Graphstyle"),this);
    connect(change_graphstyle, &QAction::triggered, this, &Plot::changeGraphStyle);
    m_menu->addAction(change_graphstyle);

    QAction* add_graph =new QAction(tr("Add Graph"),this);
    connect(add_graph, &QAction::triggered, this, &Plot::changeGraphStyle);
    m_menu->addAction(add_graph);

    QAction* clear_plot =new QAction(tr("Plot"),this);
    connect(clear_plot, &QAction::triggered, this, &Plot::clearPlot);
    m_menu->addAction(clear_plot);

    QAction* delete_plot =new QAction(tr("Delete Plot"),this);
    connect(delete_plot, &QAction::triggered, this, &Plot::deletePlot);
    m_menu->addAction(delete_plot);

    // change Graph Dialog
    m_changegpraph_dialog = new changeGraphDialog(this,parent,signal);
    Qt::WindowFlags flags = m_changegpraph_dialog->windowFlags();
    m_changegpraph_dialog->setWindowFlags(flags | Qt::Tool);

    m_ymin = 0;
    m_ymax = 0;
    m_xmin = 0;
    m_xmax = 0;
    m_plot_buffer_index = 0;
}

void Plot::ShowContextMenu(const QPoint& pos){
    qDebug() << "ShowContextMenu Position: " << pos;

    m_menu->exec(pos);
}

bool Plot::ifNameExists(QString name){
    for (int i=0; i< plottableCount(); i++){
        if (graph(i)->name().compare(name) == 0){
            return 1;
        }
    }
    return 0;
}

void Plot::changeGraphStyle(){
    m_changegpraph_dialog->show();
}

void Plot::mousePressEvent(QMouseEvent *ev){

    if(ev->button() == Qt::MouseButton::RightButton){
        m_context_menu->customContextMenuRequested(ev->globalPos());
    }
}

void Plot::clearPlot(){
    clearGraphs();
}

void Plot::addGraphToPlot(struct SettingsGraph* settings){
    m_changegpraph_dialog->addElement(settings);
}

void Plot::deletePlot(){
    deletePlot2(this->m_index);
}

void Plot::appendYData(){
    m_y_data.resize(m_y_data.size()+1);
    m_y_data[m_y_data.size()-1].resize(m_udp_buffersize);
}

void Plot::resizePlotBuffer(int udp_buffersize, int plot_buffersize){
    m_plot_buffersize = plot_buffersize;
    m_udp_buffersize = udp_buffersize;
    for (int i=0; i<m_y_data.size(); i++){
        m_y_data[i].resize(m_plot_buffersize);
    }
    m_x_data.resize(m_plot_buffersize);
}

void Plot::newData(unsigned long index){
// process new data


    if(m_signal_settings.count()>0){
        // first double is x Axis
        double x_val = m_parent->getBufferData(index,0);
        m_x_data.append(x_val);
        if(x_val > m_xmax){
            m_xmax = x_val;
        }

        int size = m_x_data.size()- m_plot_buffersize;
        for (int i= 0; i<size; i++){
            m_x_data.removeFirst();
        }
        m_xmin = m_x_data[0];
        m_xmax = m_x_data[m_x_data.size()-1];
    }

    struct Signal signal;
    for(unsigned int i=0; i<graphCount(); i++){
        signal = m_signal_settings.at(i);



        //int index_temp = (index*m_parent->getBufferCount()+signal.index);
        double y_val = m_parent->getBufferData(index, signal.index);
        m_y_data[i].append(y_val);

        int size=m_y_data[i].size()-m_plot_buffersize;
        for(int j=0; j<size; j++){
            m_y_data[i].removeFirst();
        }

        if(i==0){
            m_ymax = *std::max_element(m_y_data[i].constBegin(), m_y_data[i].constEnd());
            m_ymin = *std::min_element(m_y_data[i].constBegin(), m_y_data[i].constEnd());
        }else{

            double min = *std::min_element(m_y_data[i].constBegin(), m_y_data[i].constEnd());
            double max = *std::max_element(m_y_data[i].constBegin(), m_y_data[i].constEnd());

            if(max > m_ymax){
                m_ymax = max;
            }
            if(min < m_ymin){
                m_ymin = min;
            }

        }

        graph(i)->setData(m_x_data,m_y_data[i]);

    }

    double min_factor;
    double max_factor;
    if(m_ymin <0){
        min_factor = (m_ymin -5)/m_ymin;
    }else{
        min_factor = (m_ymin -5)/m_ymin;
    }

    if(m_ymax > 0){
        max_factor = (m_ymax+5)/m_ymax;
    }else{
        max_factor = (m_ymax +5)/m_ymax;
    }

    yAxis->setRange(m_ymin*min_factor, m_ymax*max_factor);



    xAxis->setRange(m_xmin,m_xmax);
    replot();
    update();

}

void Plot::writeJSON(QJsonObject &object){
    QJsonArray graphs;
    for(int i=0; i<graphCount(); i++){
        QJsonObject graph;
        struct SettingsGraph settings = m_changegpraph_dialog->getSettings(i);
        graph["Color"] = QString(settings.color.name());
        qDebug() << "QColor to string" <<QString(settings.color.name());
        graph["LineStyle"] = settings.linestyle;
        graph["ScatterStyle"] = settings.scatterstyle;
        graph["GraphName"] = settings.name;

        QJsonObject signal;
        signal["Datatype"] = settings.signal.datatype;
        signal["Index"] = settings.signal.index;
        signal["Signalname"] = settings.signal.name;
        signal["Offset"] = settings.signal.offset;
        graph["Signal"] = signal;

        graphs.append(graph);
    }
    if (graphs.count() > 0){
        object["Graphs"] = graphs;
    }
}

void Plot::signalsChanged(){
    m_changegpraph_dialog->updateSignals();
}

Plot::~Plot(){
    m_menu->close();
    delete m_changegpraph_dialog;
    delete m_menu;
}
