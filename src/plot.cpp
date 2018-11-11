#include "plot.h"
#include "plots.h"

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

#include <algorithm>
#include "plotscontextmenu.h"
#include "changegraphdialog.h"

Plot::Plot(Plots* plots, QWidget* parent, int index, Signals *signal):
    QCustomPlot(parent), m_index(index),m_parent(plots), m_signals(signal)
{

    connect(this, &Plot::deletePlot2, plots, &Plots::deletePlot);

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
    m_plot_buffer_index = 0;

}

void Plot::ShowContextMenu(const QPoint& pos){
    qDebug() << "ShowContextMenu Position: " << pos;

    m_menu->exec(pos);
}

void Plot::deleteGraph(struct Signal xaxis, struct Signal yaxis, int index){
    removeGraph(graph(index));

    m_parent->removeGraph(xaxis, yaxis);
}

void Plot::newGraph(struct SignalSettings settings){
    addGraph();
    struct SignalSettings dummy;
    changeGraphSettings(graphCount()-1, settings, dummy, false);
    graph(graphCount()-1)->setAdaptiveSampling(true);
}

void Plot::changeGraphSettings(int index_graph, struct SignalSettings new_settings, struct SignalSettings old_settings, bool remove_signal){
    graph(index_graph)->setLineStyle(static_cast<QCPGraph::LineStyle>(new_settings.linestyle));
    QCPScatterStyle::ScatterShape value = static_cast<QCPScatterStyle::ScatterShape>(new_settings.scatterstyle);
    graph(index_graph)->setScatterStyle(value);
    graph(index_graph)->setName(new_settings.name);
    graph(index_graph)->setPen(QPen(QColor(new_settings.color)));
    QSharedPointer<QCPGraphDataContainer> data_puffer = m_parent->getBuffer(new_settings.signal_xaxis, new_settings.signal_yaxis);
    // problem: durch das ersetzen der alten daten, wird der alte shared pointer gelöscht, aber der wurde ja bereits durch remove signals gelöscht
    graph(index_graph)->setData(data_puffer);

    if(remove_signal && new_settings.signal_xaxis.index != old_settings.signal_xaxis.index &&
            new_settings.signal_yaxis.index != old_settings.signal_yaxis.index){
        removeSignal(old_settings.signal_xaxis, old_settings.signal_yaxis);
    }
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

void Plot::addGraphToPlot(struct SignalSettings* settings){
    m_changegpraph_dialog->addElement(settings);
}

void Plot::deletePlot(){
    deletePlot2(this);
}

void Plot::newData(){
// process new data

    bool range_found;
    if(graphCount() <= 0)
        return;

    double xmin, xmax;
    QCPRange range = graph(0)->getKeyRange(range_found);
    if(range_found){ // otherwise let old range
        xmin = range.lower;
        xmax = range.upper;
        xAxis->setRange(xmin,xmax);
    }


    // y Axis
    double ymax, ymin;
    bool ranges_found = false;
    if(m_changegpraph_dialog->isAutomatic()){
        for(unsigned int i=0; i<graphCount(); i++){

            range = graph(i)->getValueRange(range_found);
            if(range_found){
                if(ranges_found==false){ // first range which was found
                    ymin = range.lower;
                    ymax = range.upper;
                    ranges_found = true;
                    continue;
                }

                if(ymax < range.upper){
                    ymax = range.upper;
                }
                if(ymin > range.lower){
                    ymin = range.lower;
                }
            }
        }
        if(ranges_found){
            if(m_changegpraph_dialog->isRelative()){
                double min_factor;
                double max_factor;
                if(ymin <0){
                    min_factor = m_changegpraph_dialog->automaticRange();
                }else if(ymin >0){
                    min_factor = 1-m_changegpraph_dialog->automaticRange();
                }else {
                    min_factor = 1;
                }

                if(ymax > 0){
                    max_factor = m_changegpraph_dialog->automaticRange();
                }else if(ymax <0){
                    max_factor = 1 - m_changegpraph_dialog->automaticRange();
                }else{
                    max_factor = 1;
                }

                yAxis->setRange(ymin*min_factor, ymax*max_factor);
            }else{
                if(ymin <0){
                    ymin -= m_changegpraph_dialog->automaticRange();
                }else { // ymin >=0
                    ymin -= m_changegpraph_dialog->automaticRange();
                }

                if(ymax >= 0){
                    ymax += m_changegpraph_dialog->automaticRange();
                }else {// ymax <0
                    ymax += m_changegpraph_dialog->automaticRange();
                }

                yAxis->setRange(ymin, ymax);
            }
        }

    }else{
        yAxis->setRange(m_changegpraph_dialog->yMin(),m_changegpraph_dialog->yMax());
    }

    replot();
    update();

}

void Plot::writeJSON(QJsonObject &object){

    struct Settings settings = m_changegpraph_dialog->getSettings();
    object["ymin"] = settings.ymin;
    object["ymax"] = settings.ymax;
    object["ifrelative_range"] = settings.ifrelative_ranging;
    object["ifautomatic_range"] = settings.ifautomatic_range;
    object["automatic_value"] = settings.automatic_value;
    QJsonArray graphs;
    for(int i=0; i<graphCount(); i++){
        struct SignalSettings signalsettings = settings.signal_settings[i];
        QJsonObject graph;

        graph["Color"] = QString(signalsettings.color.name());
        qDebug() << "QColor to string" <<QString(signalsettings.color.name());
        graph["LineStyle"] = signalsettings.linestyle;
        graph["ScatterStyle"] = signalsettings.scatterstyle;
        graph["GraphName"] = signalsettings.name;
        QJsonObject signal;
        signal["Datatype"] = signalsettings.signal_yaxis.datatype;
        signal["Index"] = signalsettings.signal_yaxis.index;
        signal["Signalname"] = signalsettings.signal_yaxis.name;
        signal["Offset"] = signalsettings.signal_yaxis.offset;
        signal["structName"] = signalsettings.signal_yaxis.struct_name;
        graph["Signal yAxis"] = signal;

        signal["Datatype"] = signalsettings.signal_xaxis.datatype;
        signal["Index"] = signalsettings.signal_xaxis.index;
        signal["Signalname"] = signalsettings.signal_xaxis.name;
        signal["Offset"] = signalsettings.signal_xaxis.offset;
        signal["structName"] = signalsettings.signal_xaxis.struct_name;
        graph["Signal xAxis"] = signal;

        graphs.append(graph);
    }
    if (graphs.count() > 0){
        object["Graphs"] = graphs;
    }
}

void Plot::importSettings(QJsonObject& plot_settings){
    if(!plot_settings.contains("Graphs")){
        qDebug() << "No plot settings found";
        return;
    }
    struct Settings settings;

    settings.ymin = plot_settings["ymin"].toDouble();
    settings.ymax = plot_settings["ymax"].toDouble();
    settings.automatic_value = plot_settings["automatic_value"].toDouble();
    settings.ifautomatic_range = plot_settings["ifautomatic_range"].toBool();
    settings.ifrelative_ranging = plot_settings["ifrelative_range"].toBool();


    QJsonArray graphs = plot_settings["Graphs"].toArray();
    for(int j=0; j<graphs.count(); j++){

        QJsonObject graph = graphs[j].toObject();

        struct SignalSettings signal_settings;
        signal_settings.name = graph["GraphName"].toString();
        signal_settings.color = QColor(graph["Color"].toString());
        signal_settings.scatterstyle = graph["ScatterStyle"].toInt();
        signal_settings.linestyle = graph["LineStyle"].toInt();

        struct Signal signal;
        QJsonObject signalsettings = graph["Signal yAxis"].toObject();
        signal.datatype = signalsettings["Datatype"].toString();
        signal.index = signalsettings["Index"].toInt();
        signal.offset = signalsettings["Offset"].toInt();
        signal.name = signalsettings["Signalname"].toString();
        signal.struct_name = signalsettings["structName"].toString();
        signal_settings.signal_yaxis = signal;

        signalsettings = graph["Signal xAxis"].toObject();
        signal.datatype = signalsettings["Datatype"].toString();
        signal.index = signalsettings["Index"].toInt();
        signal.offset = signalsettings["Offset"].toInt();
        signal.name = signalsettings["Signalname"].toString();
        signal.struct_name = signalsettings["structName"].toString();
        signal_settings.signal_xaxis = signal;

        settings.signal_settings.append(signal_settings);

        m_changegpraph_dialog->setSettings(settings);


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
