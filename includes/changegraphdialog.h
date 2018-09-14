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

struct SettingsGraph{
    QColor color;
    int linestyle;
    int scatterstyle;
    struct Signal signal;
    QString name;

};

namespace Ui {
class changeGraphDialog;
}

class changeGraphDialog : public QDialog
{
    Q_OBJECT

public:
    explicit changeGraphDialog(Plot* parent_plot, QWidget* parent, Signals *signal);
    void saveOldSettings();
    struct SettingsGraph getSettings(int index){return m_settings[index];}
    void updateSignals();

    ~changeGraphDialog();
public slots:
    void addElement(struct SettingsGraph* settings_import);
private slots:
    void apply();
    void ok();
    void cancel();
    void deleteElement();
    void addElement();
    void updateData(const QString &text);
    void updateData2();
    void listWidgetRowChanged(int row);
private:
    Ui::changeGraphDialog *ui;
    Plot* m_parent;
    QVector<struct SettingsGraph> m_settings; // old settings
    QVector<struct SettingsGraph> m_settings_new;
    int m_previous_row;

    Signals* m_signals;

};

#endif // CHANGEGRAPHDIALOG_H
