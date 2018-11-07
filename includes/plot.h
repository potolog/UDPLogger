#ifndef PLOT_H
#define PLOT_H
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

#include <QMouseEvent>
#include "signals.h"
#include "qcustomplot.h"
#include "changegraphdialog.h"

class Plots;
class changeGraphDialog;
class PlotsContextMenu;

class Plot : public QCustomPlot{
    Q_OBJECT
public:
    Plot(Plots* plots, QWidget* parent, int index, Signals* signal);
    void mousePressEvent(QMouseEvent *ev);
    void addGraphToPlot(struct SignalSettings* settings);
    bool ifNameExists(QString name);
    void deleteGraph(struct Signal xaxis,struct Signal yaxis, int index);
    void writeJSON(QJsonObject &object);
    void importSettings(QJsonObject& object);

    ~Plot();
private slots:
    void ShowContextMenu(const QPoint& pos);
    void deletePlot();
    void clearPlot();
    void changeGraphStyle();
public slots:
    void newData();
    void signalsChanged();
    void newGraph(struct SignalSettings settings);
    void changeGraphSettings(int index_graph, struct SignalSettings new_settings, struct SignalSettings old_settings, bool remove_signal);
signals:
    void deletePlot2(int index);
    void removeSignal(struct Signal xaxis,struct Signal yaxis);

private:
    int m_index;
    Plots* m_parent;
    PlotsContextMenu* m_context_menu;
    QMenu* m_menu;
    changeGraphDialog* m_changegpraph_dialog;
    Signals* m_signals;

    int m_plot_buffer_index;
    int m_plot_buffersize;
    int m_udp_buffersize;
    int buffer_index;
};

#endif // PLOT_H
