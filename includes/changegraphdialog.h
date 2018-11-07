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

#ifndef CHANGEGRAPHDIALOG_H
#define CHANGEGRAPHDIALOG_H

#include <QDialog>
#include "signals.h"

class Plot;

struct Settings{
    bool ifautomatic_range;
    bool ifrelative_ranging;
    double ymin; // manual ranging
    double ymax; // manual ranging
    double automatic_value;
    QVector<struct SignalSettings> signal_settings;
};

struct SignalSettings{
    QColor color;
    int linestyle;
    int scatterstyle;
    struct Signal signal_yaxis;
    QString name;
    struct Signal signal_xaxis;
};

namespace Ui {
class changeGraphDialog;
}

class changeGraphDialog : public QDialog
{
    Q_OBJECT

public:
    explicit changeGraphDialog(Plot* parent_plot, QWidget* parent, Signals *signal);
    //void saveOldSettings();
    struct Settings getSettings(){return m_settings_old;}
    int getSignalCount(){return m_settings_new.signal_settings.count();}
    void updateSignals();
    bool isRelative(){return m_settings_new.ifrelative_ranging;}
    bool isAutomatic(){return m_settings_new.ifautomatic_range;}
    double yMin(){return m_settings_new.ymin;}
    double yMax(){return m_settings_new.ymax;}
    double automaticRange(){return m_settings_new.automatic_value;}
    void setSettings(struct Settings& settings);

    ~changeGraphDialog();
signals:
    void newGraph(struct SignalSettings);
    void changeGraphSettings(int index_graph, struct SignalSettings new_settings, struct SignalSettings old_settings, bool remove_signal);
public slots:
    void addElement(struct SignalSettings* settings_import);
private slots:
    void apply();
    void ok();
    void cancel();
    void deleteElement();
    void addElement();
    void updateData(const QString &text);
    void updateData2();
    void listWidgetRowChanged(int row);
    void on_spinbox_y_min_valueChanged(double arg1);
    void on_spinbox_range_adjustment_valueChanged(double arg1);
    void on_spinbox_y_max_valueChanged(double arg1);
    void on_checkbox_if_automatic_range_toggled(bool checked);
    void on_rb_relative_toggled(bool checked);

private:
    Ui::changeGraphDialog *ui;
    Plot* m_parent;
    struct Settings m_settings_old;
    struct Settings m_settings_new;
    int m_previous_row;

    Signals* m_signals;

};

#endif // CHANGEGRAPHDIALOG_H
