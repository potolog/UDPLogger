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

    ui->spinbox_y_max->setRange(-4294967296,4294967296);
    ui->spinbox_y_max->setDecimals(3);
    ui->spinbox_y_min->setRange(-4294967296,4294967296);
    ui->spinbox_y_min->setDecimals(3);

    ui->spinbox_range_adjustment->setRange(0,4294967296);



    updateSignals();

    disableSignalSettings(true);

    m_previous_row = -1;

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &changeGraphDialog::listWidgetRowChanged);
    connect(ui->txt_name, &QTextEdit::textChanged, this, &changeGraphDialog::updateData2);
    connect(ui->pb_cancel, &QPushButton::clicked, this, &changeGraphDialog::cancel);
    connect(ui->pb_apply, &QPushButton::clicked, this, &changeGraphDialog::apply);
    connect(ui->pb_ok, &QPushButton::clicked, this, &changeGraphDialog::ok);
    connect(ui->pb_delete, &QPushButton::clicked, this, &changeGraphDialog::deleteElement);
    connect(ui->pb_add, &QPushButton::clicked, this, qOverload<>(&changeGraphDialog::addElement));

    connect(this,&changeGraphDialog::newGraph, m_parent, &Plot::newGraph);
    connect(this,&changeGraphDialog::changeGraphSettings, m_parent, &Plot::changeGraphSettings);

    ui->checkbox_if_automatic_range->setChecked(false);
    m_settings_old.ifautomatic_range = false;
    ui->spinbox_range_adjustment->setValue(100);
    m_settings_old.automatic_value = 100/100;
    ui->spinbox_y_max->setValue(0);
    m_settings_old.ymax = 0;
    ui->spinbox_y_min->setValue(0);
    m_settings_old.ymin =0;
    ui->rb_relative->setChecked(true);
    m_settings_old.ifrelative_ranging = true;

    m_settings_new = m_settings_old;



}

void changeGraphDialog::disableSignalSettings(bool disable){
    ui->combo_color->setEnabled(!disable);
    ui->combo_linestyle->setEnabled(!disable);
    ui->combo_scatter_style->setEnabled(!disable);
    ui->txt_name->setEnabled(!disable);
    ui->combo_signalname->setEnabled(!disable);
    ui->combo_signalname_xaxis->setEnabled(!disable);
}

void changeGraphDialog::listWidgetRowChanged(int row){

    if(row <0){
        return;
    }
    if (m_previous_row >= 0){
        m_settings_new.signal_settings[m_previous_row].color = QColor(static_cast<Qt::GlobalColor>(ui->combo_color->currentData().toInt()));
        m_settings_new.signal_settings[m_previous_row].linestyle = ui->combo_linestyle->currentData().toInt();
        m_settings_new.signal_settings[m_previous_row].scatterstyle = ui->combo_scatter_style->currentData().toInt();
        m_settings_new.signal_settings[m_previous_row].name =ui->txt_name->toPlainText();
        m_settings_new.signal_settings[m_previous_row].signal_yaxis = m_signals->getSignal(ui->combo_signalname->currentData().toInt());
    }
    m_previous_row = row;
    for (int i=0; i<ui->combo_color->count(); i++){
        if (QColor(static_cast<Qt::GlobalColor>(ui->combo_color->itemData(i).toInt())) == m_settings_new.signal_settings[row].color){
            ui->combo_color->setCurrentIndex(i);
            break;
        }
    }

    for (int i=0; i<ui->combo_linestyle->count(); i++){
        if (ui->combo_linestyle->itemData(i).value<int>() == m_settings_new.signal_settings[row].linestyle){
            ui->combo_linestyle->setCurrentIndex(i);
            break;
        }
    }

    for (int i=0; i<ui->combo_scatter_style->count(); i++){
        if (ui->combo_scatter_style->itemData(i).value<int>() == m_settings_new.signal_settings[row].scatterstyle){
            ui->combo_scatter_style->setCurrentIndex(i);
            break;
        }
    }

    for (int i=0; i<ui->combo_signalname->count(); i++){
        if (m_signals->getSignal(i).name.compare(m_settings_new.signal_settings[row].signal_yaxis.name)==0){
            ui->combo_signalname->setCurrentIndex(i);
            break;
        }
    }

    ui->txt_name->setText(m_settings_new.signal_settings[row].name);
}

void changeGraphDialog::updateData(const QString &text){

    struct SignalSettings settings;


    int row = ui->listWidget->currentRow();

    if (row > 0){
        m_settings_new.signal_settings[row].linestyle = ui->combo_linestyle->currentData().toInt();
        m_settings_new.signal_settings[row].scatterstyle = ui->combo_scatter_style->currentData().toInt();
        m_settings_new.signal_settings[row].color = ui->combo_color->currentData().value<QColor>();
    }

}

void changeGraphDialog::updateData2(){
    int row = ui->listWidget->currentRow();

    if (row >= 0){
        m_settings_new.signal_settings[row].name = ui->txt_name->toPlainText();
        ui->listWidget->item(row)->setText(ui->txt_name->toPlainText());
    }

}

void changeGraphDialog::cancel(){
    // restore old data
    for (int i=0; i<m_settings_old.signal_settings.length(); i++){
        m_parent->graph(i)->setLineStyle(static_cast<QCPGraph::LineStyle>(m_settings_old.signal_settings[i].linestyle));
        m_parent->graph(i)->setScatterStyle(static_cast<QCPScatterStyle::ScatterShape>(m_settings_old.signal_settings[i].scatterstyle));
        m_parent->graph(i)->setName(m_settings_old.signal_settings[i].name);
        m_parent->graph(i)->setPen(QPen(QColor(m_settings_old.signal_settings[i].color)));
    }

    m_settings_new = m_settings_old;
    close();
}

void changeGraphDialog::apply(){
    // remove deleted graphs!!!

    int row = ui->listWidget->currentRow();
    if(row >= 0){
        m_settings_new.signal_settings[row].color = QColor(static_cast<Qt::GlobalColor>(ui->combo_color->currentData().toInt()));
        m_settings_new.signal_settings[row].linestyle = ui->combo_linestyle->currentData().toInt();
        m_settings_new.signal_settings[row].scatterstyle = ui->combo_scatter_style->currentData().toInt();
        m_settings_new.signal_settings[row].name = ui->txt_name->toPlainText();
        m_settings_new.signal_settings[row].signal_xaxis = m_signals->getSignal(ui->combo_signalname_xaxis->currentData().toInt());
        m_settings_new.signal_settings[row].signal_yaxis = m_signals->getSignal(ui->combo_signalname->currentData().toInt());
        m_settings_new.ymin = ui->spinbox_y_min->value();
        m_settings_new.ymax = ui->spinbox_y_max->value();
        m_settings_new.automatic_value = ui->spinbox_range_adjustment->value();
        m_settings_new.ifautomatic_range = ui->checkbox_if_automatic_range->isChecked();
        m_settings_new.ifrelative_ranging = ui->rb_relative->isChecked();
    }


    for (int i=0; i<m_settings_old.signal_settings.length(); i++){
        emit changeGraphSettings(i,m_settings_new.signal_settings[i],m_settings_old.signal_settings[i],true);
    }


    m_settings_old = m_settings_new;
}

void changeGraphDialog::ok(){
    apply();
    close();
}

// at the moment directly deleted, not possible to get back
void changeGraphDialog::deleteElement(){

    int row = ui->listWidget->currentRow();
    QListWidgetItem* item = ui->listWidget->takeItem(row); // removes the element from the list and returns a pointer to the item to delete
    m_previous_row = -1;
    delete item;
    struct Signal xaxis = m_settings_old.signal_settings.at(row).signal_xaxis;
    struct Signal yaxis = m_settings_old.signal_settings.at(row).signal_yaxis;
    m_settings_old.signal_settings.remove(row);
    m_settings_new.signal_settings.remove(row);

    m_parent->deleteGraph(xaxis, yaxis,row);

    if(ui->listWidget->count() <= 0){
        disableSignalSettings(true);
    }
}

void changeGraphDialog::addElement(){
    addElement(nullptr);
}

void changeGraphDialog::addElement(struct SignalSettings* settings_import=nullptr){
    struct SignalSettings settings;
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
        settings.signal_yaxis = m_signals->getSignal(ui->combo_signalname->itemData(0).toInt());
        settings.signal_xaxis = m_signals->getSignal(ui->combo_signalname_xaxis->itemData(0).toInt());
    }else{
        settings.linestyle = settings_import->linestyle;
        settings.scatterstyle = settings_import->scatterstyle;
        settings.color = settings_import->color;
        settings.name = settings_import->name;
        settings.signal_yaxis = settings_import->signal_yaxis;
        settings.signal_xaxis = settings_import->signal_xaxis;
    }


    emit newGraph(settings);

    m_settings_old.signal_settings.append(settings);
    m_settings_new.signal_settings.append(settings);

    ui->listWidget->addItem(settings.name);
    ui->listWidget->setCurrentRow(ui->listWidget->count()-1);
    ui->combo_signalname->setCurrentText(settings.signal_yaxis.name);
    ui->combo_signalname_xaxis->setCurrentText(settings.signal_xaxis.name);

    disableSignalSettings(false);
}

void changeGraphDialog::updateSignals(){
    ui->combo_signalname->clear();
    ui->combo_signalname_xaxis->clear();
    struct Signal signal_temp;
    for (int i=0; i< m_signals->getSignalCount(); i++){
        signal_temp = m_signals->getSignal(i);
        ui->combo_signalname->addItem(signal_temp.name,i);
        ui->combo_signalname_xaxis->addItem(signal_temp.name,i);
    }

    if(m_settings_old.signal_settings.length() <1){
        return;
    }
    // delete all signals if signal for x axis does not exist
    if(!m_signals->signalExist(m_settings_old.signal_settings.at(0).signal_xaxis)){
        int count = ui->listWidget->count();
        for(int i=0; i< count; i++){
            ui->listWidget->setCurrentRow(0);
            deleteElement();
        }
        return;
    }

    // delete all signals where the signal does not exist anymore
    int i = 0;
    while(ui->listWidget->currentRow() != ui->listWidget->count()){
        if(i >= m_settings_old.signal_settings.length()){
            break;
        }
        if(!m_signals->signalExist(m_settings_old.signal_settings.at(i).signal_yaxis)){
            ui->listWidget->setCurrentRow(i);
            deleteElement();
            continue;
        }
        i++;
    }

}

changeGraphDialog::~changeGraphDialog()
{
    delete ui;
}

void changeGraphDialog::on_spinbox_y_min_valueChanged(double arg1)
{
    m_settings_new.ymin = arg1;
}

void changeGraphDialog::on_spinbox_range_adjustment_valueChanged(double arg1)
{
    m_settings_new.automatic_value = arg1;
    if(isRelative()){
        m_settings_new.automatic_value /= 100;
    }
}

void changeGraphDialog::on_spinbox_y_max_valueChanged(double arg1)
{
    m_settings_new.ymax = arg1;
}

void changeGraphDialog::on_checkbox_if_automatic_range_toggled(bool checked)
{
    m_settings_new.ifautomatic_range = checked;
}

void changeGraphDialog::on_rb_relative_toggled(bool checked)
{
    m_settings_new.ifrelative_ranging = checked;

    if(!checked){
        m_settings_new.automatic_value*=100;
    }
}

void changeGraphDialog::setSettings(struct Settings& settings){

    ui->checkbox_if_automatic_range->setChecked(settings.ifautomatic_range);
    ui->rb_relative->setChecked(settings.ifrelative_ranging);
    ui->rb_absolute->setChecked(!settings.ifrelative_ranging);
    ui->spinbox_y_min->setValue(settings.ymin);
    ui->spinbox_y_max->setValue(settings.ymax);
    ui->spinbox_range_adjustment->setValue(settings.automatic_value);
    for(int i=0; i< settings.signal_settings.length(); i++){
        addElement(&settings.signal_settings[i]);
    }
}
