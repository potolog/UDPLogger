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

#include "changegraphdialog.h"
#include "ui_changegraphdialog.h"
#include "qcustomplot.h"
#include "plot.h"
#include "signals.h"

changeGraphDialog::changeGraphDialog(Plot *parent_plot, QWidget* parent, Signals* signal) :
    QDialog(parent),
    ui(new Ui::changeGraphDialog), m_parent(parent_plot), m_signals(signal)
{
    ui->setupUi(this);

    ui->combo_color->addItem("Qt::blue",static_cast<int>(Qt::blue));
    ui->combo_color->addItem("Qt::black",static_cast<int>(Qt::black));
    ui->combo_color->addItem("Qt::red",static_cast<int>(Qt::red));
    ui->combo_color->addItem("Qt::darkred",static_cast<int>(Qt::darkRed));
    ui->combo_color->addItem("Qt::green",static_cast<int>(Qt::green));
    ui->combo_color->addItem("Qt::darkBlue",static_cast<int>(Qt::darkBlue));
    ui->combo_color->addItem("Qt::cyan",static_cast<int>(Qt::cyan));
    ui->combo_color->addItem("Qt::magenta",static_cast<int>(Qt::magenta));
    ui->combo_color->addItem("Qt::yellow",static_cast<int>(Qt::yellow));
    ui->combo_color->addItem("Qt::darkYellow",static_cast<int>(Qt::darkYellow));
    ui->combo_color->addItem("Qt::gray",static_cast<int>(Qt::gray));
    ui->combo_color->addItem("Qt::darkGray",static_cast<int>(Qt::darkGray));

    ui->combo_scatter_style->addItem("QCPScatterStyle::ssNone",QCPScatterStyle::ssNone);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssDot",QCPScatterStyle::ssDot);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssCross",QCPScatterStyle::ssCross);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssPlus",QCPScatterStyle::ssPlus);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssCircle",QCPScatterStyle::ssCircle);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssDisc",QCPScatterStyle::ssDisc);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssSquare",QCPScatterStyle::ssSquare);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssDiamond",QCPScatterStyle::ssDiamond);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssStar",QCPScatterStyle::ssStar);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssTriangle",QCPScatterStyle::ssTriangle);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssTriangleInverted",QCPScatterStyle::ssTriangleInverted);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssCrossSquare",QCPScatterStyle::ssCrossSquare);
    ui->combo_scatter_style->addItem("QCPScatterStyle::ssPlusSquare",QCPScatterStyle::ssPlusSquare);

    ui->combo_linestyle->addItem("QCPGraph::LineStyle::lsNone",QCPGraph::LineStyle::lsNone);
    ui->combo_linestyle->addItem("QCPGraph::LineStyle::lsLine",QCPGraph::LineStyle::lsLine);
    ui->combo_linestyle->addItem("QCPGraph::LineStyle::lsStepLeft",QCPGraph::LineStyle::lsStepLeft);
    ui->combo_linestyle->addItem("QCPGraph::LineStyle::lsStepRight",QCPGraph::LineStyle::lsStepRight);
    ui->combo_linestyle->addItem("QCPGraph::LineStyle::lsStepCenter",QCPGraph::LineStyle::lsStepCenter);
    ui->combo_linestyle->addItem("QCPGraph::LineStyle::lsImpulse",QCPGraph::LineStyle::lsImpulse);

    updateSignals();

    if (ui->listWidget->count() <= 0){
        ui->combo_color->setEnabled(false);
        ui->combo_linestyle->setEnabled(false);
        ui->combo_scatter_style->setEnabled(false);
        ui->txt_name->setEnabled(false);
        ui->combo_signalname->setEnabled(false);
    }

    m_previous_row = -1;

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &changeGraphDialog::listWidgetRowChanged);
    connect(ui->txt_name, &QTextEdit::textChanged, this, &changeGraphDialog::updateData2);
    connect(ui->pb_cancel, &QPushButton::clicked, this, &changeGraphDialog::cancel);
    connect(ui->pb_apply, &QPushButton::clicked, this, &changeGraphDialog::apply);
    connect(ui->pb_ok, &QPushButton::clicked, this, &changeGraphDialog::ok);
    connect(ui->pb_delete, &QPushButton::clicked, this, &changeGraphDialog::deleteElement);
    connect(ui->pb_add, &QPushButton::clicked, this, qOverload<>(&changeGraphDialog::addElement));

}
void changeGraphDialog::listWidgetRowChanged(int row){

    if(row <0){
        return;
    }
    if (m_previous_row >= 0){
        m_settings_new[m_previous_row].color = QColor(static_cast<Qt::GlobalColor>(ui->combo_color->currentData().toInt()));
        m_settings_new[m_previous_row].linestyle = ui->combo_linestyle->currentData().toInt();
        m_settings_new[m_previous_row].scatterstyle = ui->combo_scatter_style->currentData().toInt();
        m_settings_new[m_previous_row].name =ui->txt_name->toPlainText();
        m_settings_new[m_previous_row].signal = m_signals->getSignal(ui->combo_signalname->currentData().toInt());
    }
    m_previous_row = row;
    for (int i=0; i<ui->combo_color->count(); i++){
        if (QColor(static_cast<Qt::GlobalColor>(ui->combo_color->itemData(i).toInt())) == m_settings_new[row].color){
            ui->combo_color->setCurrentIndex(i);
            break;
        }
    }

    for (int i=0; i<ui->combo_linestyle->count(); i++){
        if (ui->combo_linestyle->itemData(i).value<int>() == m_settings_new[row].linestyle){
            ui->combo_linestyle->setCurrentIndex(i);
            break;
        }
    }

    for (int i=0; i<ui->combo_scatter_style->count(); i++){
        if (ui->combo_scatter_style->itemData(i).value<int>() == m_settings_new[row].scatterstyle){
            ui->combo_scatter_style->setCurrentIndex(i);
            break;
        }
    }

    for (int i=0; i<ui->combo_signalname->count(); i++){
        if (m_signals->getSignal(i).name.compare(m_settings_new[row].signal.name)==0){
            ui->combo_signalname->setCurrentIndex(i);
            break;
        }
    }

    ui->txt_name->setText(m_settings_new[row].name);
}

void changeGraphDialog::updateData(const QString &text){

    struct SettingsGraph settings;


    int row = ui->listWidget->currentRow();

    if (row > 0){
        m_settings_new[row].linestyle = ui->combo_linestyle->currentData().toInt();
        m_settings_new[row].scatterstyle = ui->combo_scatter_style->currentData().toInt();
        m_settings_new[row].color = ui->combo_color->currentData().value<QColor>();
    }

}

void changeGraphDialog::updateData2(){
    int row = ui->listWidget->currentRow();

    if (row >= 0){
        m_settings_new[row].name = ui->txt_name->toPlainText();
        ui->listWidget->item(row)->setText(ui->txt_name->toPlainText());
    }

}

// Save all settings if cancel was pressed
void changeGraphDialog::saveOldSettings(){

    m_settings.clear();

    struct SettingsGraph settings;


    for (int i=0; i < m_parent->plottableCount(); i++){
        settings.linestyle = m_parent->graph(i)->lineStyle();
        settings.scatterstyle = m_parent->graph(i)->scatterStyle().shape();
        settings.color = m_parent->graph(i)->pen().color();
        settings.name = m_parent->graph(i)->name();

        m_settings.append(settings);
    }
}

void changeGraphDialog::cancel(){
    // restore old data
    for (int i=0; i<m_settings.length(); i++){
        m_parent->graph(i)->setLineStyle(static_cast<QCPGraph::LineStyle>(m_settings[i].linestyle));
        m_parent->graph(i)->setScatterStyle(static_cast<QCPScatterStyle::ScatterShape>(m_settings[i].scatterstyle));
        m_parent->graph(i)->setName(m_settings[i].name);
        m_parent->graph(i)->setPen(QPen(QColor(m_settings[i].color)));
    }

    for (int i=0; i<m_settings.length(); i++){
        m_settings_new[i].name = m_settings[i].name;
        m_settings_new[i].color = m_settings[i].color;
        m_settings_new[i].linestyle = m_settings[i].linestyle;
        m_settings_new[i].scatterstyle = m_settings[i].scatterstyle;
    }
    close();
}

void changeGraphDialog::apply(){
    // remove deleted graphs!!!

    int row = ui->listWidget->currentRow();
    if(row >= 0){
        m_settings_new[row].color = QColor(static_cast<Qt::GlobalColor>(ui->combo_color->currentData().toInt()));
        m_settings_new[row].linestyle = ui->combo_linestyle->currentData().toInt();
        m_settings_new[row].scatterstyle = ui->combo_scatter_style->currentData().toInt();
        m_settings_new[row].name =ui->txt_name->toPlainText();
        m_settings_new[row].signal = m_signals->getSignal(ui->combo_signalname->currentData().toInt());
    }


    for (int i=0; i<m_settings.length(); i++){
        m_parent->graph(i)->setLineStyle(static_cast<QCPGraph::LineStyle>(m_settings_new[i].linestyle));
        QCPScatterStyle::ScatterShape value = static_cast<QCPScatterStyle::ScatterShape>(m_settings_new[i].scatterstyle);
        m_parent->graph(i)->setScatterStyle(value);
        m_parent->graph(i)->setName(m_settings_new[i].name);
        m_parent->graph(i)->setPen(QPen(QColor(m_settings_new[i].color)));
        m_parent->getSignalSettings()->replace(i,m_settings_new[i].signal);
    }


    struct SettingsGraph settings;
    m_settings.clear();
    // save new settings into m_settings
    for (int i=0; i < m_parent->plottableCount(); i++){
        settings.linestyle = m_settings_new[i].linestyle;
        settings.scatterstyle = m_settings_new[i].scatterstyle;
        settings.color = m_settings_new[i].color;
        settings.name = m_settings_new[i].name;
        settings.signal = m_settings_new[i].signal;

        m_settings.append(settings);
    }
}

void changeGraphDialog::ok(){
    apply();
    close();
}

void changeGraphDialog::deleteElement(){

    int row = ui->listWidget->currentRow();
    QListWidgetItem* item = ui->listWidget->takeItem(row); // removes the element from the list and returns a pointer to the item to delete
    m_previous_row = -1;
    delete item;
    m_settings.remove(row);
    m_settings_new.remove(row);
    m_parent->getSignalSettings()->remove(row);
    m_parent->removeData(row);
    m_parent->removeGraph(m_parent->graph(row));
}

void changeGraphDialog::addElement(){
    addElement(nullptr);
}

void changeGraphDialog::addElement(struct SettingsGraph* settings_import=nullptr){
    struct SettingsGraph settings;
    QString name = "";

    if (settings_import == nullptr){
        QString temp_name;
        int i = 0;
        while(name.compare("") == 0){
            bool name_exist = 0;
            i++;
            temp_name = "Default Name" + QString::number(i);
            for (int j=0; j< ui->listWidget->count(); j++){
                if(ui->listWidget->item(j)->text().compare(temp_name)==0){
                    name_exist = 1;
                    break;
                }
            }
            if (!name_exist){
                name = temp_name;
            }
        }


        settings.linestyle = ui->combo_linestyle->itemData(1).toInt();
        settings.scatterstyle = ui->combo_scatter_style->itemData(0).toInt();
        settings.color = QColor(static_cast<Qt::GlobalColor>(ui->combo_color->itemData(0).toInt()));
        settings.name = name;
        if(m_signals->getSignalCount() == 0){
            QMessageBox msgBox;
            msgBox.setText(tr("No signals available"));
            msgBox.exec();
            return;
        }
        settings.signal = m_signals->getSignal(ui->combo_signalname->itemData(0).toInt());
    }else{
        settings.linestyle = settings_import->linestyle;
        settings.scatterstyle = settings_import->scatterstyle;
        settings.color = settings_import->color;
        settings.name = settings_import->name;
        settings.signal = settings_import->signal;
    }
    ui->listWidget->addItem(settings.name);
    m_parent->getSignalSettings()->append(settings.signal);
    m_parent->addGraph();

    m_parent->graph(m_parent->graphCount()-1)->setLineStyle(static_cast<QCPGraph::LineStyle>(settings.linestyle));
    QCPScatterStyle::ScatterShape value = static_cast<QCPScatterStyle::ScatterShape>(settings.scatterstyle);
    m_parent->graph(m_parent->graphCount()-1)->setScatterStyle(value);
    m_parent->graph(m_parent->graphCount()-1)->setName(settings.name);
    m_parent->graph(m_parent->graphCount()-1)->setPen(QPen(QColor(settings.color)));


    m_parent->appendYData();

    m_settings.append(settings);
    m_settings_new.append(settings);

    ui->listWidget->setCurrentRow(ui->listWidget->count()-1);

    ui->combo_color->setEnabled(true);
    ui->combo_linestyle->setEnabled(true);
    ui->combo_scatter_style->setEnabled(true);
    ui->txt_name->setEnabled(true);
    ui->combo_signalname->setEnabled(true);
}

void changeGraphDialog::updateSignals(){
    struct Signal signal_temp;
    for (int i=0; i< m_signals->getSignalCount(); i++){
        signal_temp = m_signals->getSignal(i);
        ui->combo_signalname->addItem(signal_temp.name,i);
    }
}

changeGraphDialog::~changeGraphDialog()
{
    delete ui;
}
