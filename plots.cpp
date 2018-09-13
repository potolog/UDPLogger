#include "plots.h"
#include <QDebug>
#include "changegraph.h"
#include "plotscontextmenu.h"
#include "udp.h"
#include "signals.h"
#include <algorithm>
#include <QFileDialog>
#include <QMessageBox>
#include "settingsdialog.h"


Plot::Plot(Plots* plots, QWidget* parent, int buffersize_plot,int buffersize_udp, int index, Signals *signal):
    QCustomPlot(parent), m_index(index),m_parent(plots)
{

    connect(this, &Plot::deletePlot2, plots, &Plots::deletePlot);

    resizePlotBuffer(buffersize_udp, buffersize_plot);
    m_context_menu = new PlotsContextMenu();
    m_context_menu->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_context_menu, &PlotsContextMenu::customContextMenuRequested,this,&Plot::ShowContextMenu);


    m_menu = new QMenu;


    QAction* change_graphstyle =new QAction(tr("Change Graphstyle"));
    connect(change_graphstyle, &QAction::triggered, this, &Plot::changeGraphStyle);
    m_menu->addAction(change_graphstyle);

    QAction* add_graph =new QAction(tr("Add Graph"));
    connect(add_graph, &QAction::triggered, this, &Plot::changeGraphStyle);
    m_menu->addAction(add_graph);

    QAction* clear_plot =new QAction(tr("Plot"));
    connect(clear_plot, &QAction::triggered, this, &Plot::clearPlot);
    m_menu->addAction(clear_plot);

    QAction* delete_plot =new QAction(tr("Delete Plot"));
    connect(delete_plot, &QAction::triggered, this, &Plot::deletePlot);
    m_menu->addAction(delete_plot);

    // change Graph Dialog
    m_changegpraph_dialog = new changeGraph(this,parent,signal);
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

Plots::Plots(QWidget *parent, Signals* signal): m_parent(parent), m_signals(signal)
{
    m_program_version = "1.0";
    m_project_name = "Project1";
    m_layout = new QVBoxLayout(parent);
    m_index_buffer = 0;
    m_ifudpLogging = false;

    int port = 60000;
    int udp_buffersize = 400;
    int plot_buffersize = 200;
    m_buffersize_plot = plot_buffersize;
    m_buffersize_udp = udp_buffersize;

    changeDataBufferSize(m_buffersize_plot,m_buffersize_udp);
    m_port = port;
    m_hostaddress = QHostAddress::Any;

    m_mutex = new QMutex;
    m_udp = new UDP(this, m_mutex);
    m_udp->init();

    m_udp->moveToThread(&m_udp_thread);
    m_udp_thread.start();
    m_ifudpLogging = 1;
    m_udp->connectDataReady();

    m_settings_dialog = new SettingsDialog(this);
    m_settings_dialog->setSettings(m_project_name, m_hostaddress, m_buffersize_udp, m_buffersize_plot, m_port);
}

void Plots::newData(){
    if(m_index_buffer >= m_buffersize_datas){
        m_index_buffer = 0;
    }
    m_mutex->lock();
    struct Signal signal;
    for (int i=0; i<m_signals->getSignalCount(); i++){
        signal = m_signals->getSignal(i);

        double value;
        if(signal.datatype.compare("float")== 0){
            float temp =*m_udp->getFloatPointer(signal.offset);
            value = static_cast<double>(temp);

        }else if(signal.datatype.compare("double")==0){
            value = *m_udp->getDoublePointer(signal.offset);

        }else if(signal.datatype.compare("char")==0){
            char temp = *m_udp->getCharPointer(signal.offset);
            value = static_cast<double>(temp);

        }else if(signal.datatype.compare("int")==0){
            int temp = *m_udp->getIntPointer(signal.offset);
            value = static_cast<double>(temp);
        }else{
            value = 0; // not defined
        }

        m_buffer[m_index_buffer][i] = value;
    }
    m_mutex->unlock();
    emit newData2(m_index_buffer);

    m_index_buffer++;

}

void Plots::deletePlot(int index){
    qDebug() << "Index: " << index;
    //m_layout->removeWidget(m_plots.at(index));
    // disconnecten!
    QLayoutItem* item;
    item = m_layout->takeAt( index );
    if(item->widget() == m_plots.at(index)){
        qDebug() << "Deleting Object";
        delete item->widget();
        m_plots.remove(index);
    }
}

void Plots::exportSettings(){
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Save Address Book"), "/home",
            tr("UDP Logger Config Files (*.udp_loggerSettings)"));


    QJsonObject object;

    object["ProjectName"] = m_project_name;
    object["ProgramVersion"] = m_program_version;

    QJsonArray plots;
    for (int i=0; i< m_plots.length(); i++){
        QJsonObject plotObject;
        m_plots[i]->writeJSON(plotObject);
        plots.append(plotObject);
     }
    object["Plots"] = plots;

    QJsonArray active_signals;
    for (int i=0; i< m_signals->getSignalCount();i++){
        QJsonObject signal;
        signal["Datatype"] = m_signals->getSignal(i).datatype;
        signal["Index"] = m_signals->getSignal(i).index;
        signal["Offset"]  = m_signals->getSignal(i).offset;
        signal["Signalname"]  = m_signals->getSignal(i).name;
        active_signals.append(signal);
    }
    object["Signals"] = active_signals;

    QJsonObject network_settings;
    network_settings["HostAddress"] = m_hostaddress.toString();
    network_settings["Port"] = m_port;
    network_settings["UDPPufferSize"] = m_buffersize_udp;
    object["NetworkSettings"] = network_settings;
    object["PlotPufferSize"] = m_buffersize_plot;
    object["DataPufferSize"] = m_buffersize_datas;

    QFile saveFile(fileName);

   if (!saveFile.open(QIODevice::WriteOnly)) {
       qWarning("Couldn't open save file.");
       //return false;
   }

   QJsonDocument document(object);
   saveFile.write(document.toJson());

}

void Plots::importSettings(){
    // before importing remove everything!!!!

    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open settingsfile"), "/home", tr("UDP Logger Config Files (*.udp_loggerSettings)"));

    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

    QJsonObject object = d.object();

    if(object.contains("Signals")){
        QJsonArray used_signals = object["Signals"].toArray();
        QVector<struct Signal> signal_vector;
        struct Signal signal_element;

        for(int i=0; i<used_signals.count(); i++){
            QJsonObject signal = used_signals[i].toObject();
            signal_element.datatype = signal["Datatype"].toString();
            signal_element.index = signal["Index"].toInt();
            signal_element.offset = signal["Offset"].toInt();
            signal_element.name = signal["Signalname"].toString();
            signal_vector.append(signal_element);
        }
        m_signals->setSignals(&signal_vector);
    }

    if(object.contains("Plots")){
        QJsonArray plots = object["Plots"].toArray();

        for(int i=0; i<plots.count(); i++){
            createNewPlot();

            QJsonObject plot = plots[i].toObject();
            if(plot.contains("Graphs")){
                QJsonArray graphs = plot["Graphs"].toArray();
                for(int j=0; j<graphs.count(); j++){

                    QJsonObject graph = graphs[j].toObject();
                    if (graph.contains("Color")&&graph.contains("GraphName")&&
                            graph.contains("LineStyle") && graph.contains("ScatterStyle")&&
                            graph.contains("Signal")){
                        struct SettingsGraph settings;
                        settings.name = graph["GraphName"].toString();
                        settings.color = QColor(graph["Color"].toString());
                        settings.scatterstyle = graph["ScatterStyle"].toInt();
                        settings.linestyle = graph["LineStyle"].toInt();

                        QJsonObject signal_settings = graph["Signal"].toObject();

                        struct Signal signal;
                        signal.datatype = signal_settings["Datatype"].toString();
                        signal.index = signal_settings["Index"].toInt();
                        signal.offset = signal_settings["Offset"].toInt();
                        signal.name = signal_settings["Signalname"].toString();
                        settings.signal = signal;

                        m_plots[i]->addGraphToPlot(&settings);
                    }else{
                        qDebug() << "Some settings not found";
                    }



                }
            }
        }
    }

    if(object.contains("NetworkSettings") && object.contains("PlotPufferSize") && object.contains("ProjectName")){
        m_buffersize_plot = object["PlotPufferSize"].toInt();
        QJsonObject network_settings = object["NetworkSettings"].toObject();
        m_hostaddress = QHostAddress(network_settings["HostAddress"].toString());
        m_port = network_settings["Port"].toInt();
        m_buffersize_udp = network_settings["UDPPufferSize"].toInt();
        m_project_name = object["ProjectName"].toString();

        m_settings_dialog->setSettings(m_project_name, m_hostaddress,m_buffersize_udp,m_buffersize_plot,m_port);
        emit resizePlotBuffer(m_buffersize_udp, m_buffersize_plot);
    }

    if(object.contains("DataPufferSize")){
        m_buffersize_datas = object["DataPufferSize"].toInt();
        changeDataBufferSize(m_buffersize_datas,m_buffersize_udp);
    }
   }

Plot* Plots::createNewPlot()
{
    Plot* plot = new Plot(this, m_parent, m_buffersize_plot,m_buffersize_udp, m_plots.length(),m_signals);
    connect(this, &Plots::resizePlotBuffer, plot, &Plot::resizePlotBuffer);
    m_plots.append(plot);
    m_layout->addWidget(plot);
    connect(this, &Plots::newData2, plot, &Plot::newData);
    return plot;
}

void Plots::startUDP(){
    m_udp->connectDataReady();
    m_ifudpLogging = 1;
}

void Plots::stopUDP(){
    m_udp->disconnectDataReady();
    m_ifudpLogging=0;
}

void Plots::settings(){
    if(m_ifudpLogging){
        QMessageBox msgBox;
        msgBox.setText("Please Stop UDP Logging");
        msgBox.exec();

        return;
    }

    m_settings_dialog->exec();
}

void Plots::settingsAccepted(QString project_name, QHostAddress hostname, int buffersize_udp, int buffersize_plot, int port){
    m_project_name = project_name;
    m_hostaddress = hostname;
    m_buffersize_udp = buffersize_udp;
    m_buffersize_plot = buffersize_plot;
    m_port = port;

    m_udp->init(hostname,static_cast<quint16>(port),buffersize_udp);
}

void Plots::changeDataBufferSize(int size, int size_udp){
    m_buffersize_datas=size;
    m_buffersize_udp = size_udp;
    m_buffer.resize(size);

    for(int i=0; i<size; i++){
        m_buffer[i].resize(size_udp);
    }
}

Plots::~Plots(){
    m_udp_thread.quit();
    m_udp_thread.wait(); // waiting till thread quits
    delete m_udp;
    delete m_mutex;
}
