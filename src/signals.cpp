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

#include "signals.h"
#include <QFile>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <xlnt/xlnt.hpp>
#include <iostream>




Signals::Signals(){
    // nur zum testen
        struct Signal signal;

        signal.name = "double a";
        signal.datatype = "double";
        signal.offset = 0;
        signal.index = 0;
        m_signals.append(signal);

        signal.name = "float c";
        signal.datatype = "float";
        signal.offset = 8;
        signal.index = 1;
        m_signals.append(signal);

        signal.name = "inta";
        signal.datatype = "int";
        signal.offset = 12; // anzahl bytes offset
        signal.index = 2;
        m_signals.append(signal);
}

void Signals::importJSonFile(QString filename){

    QFile file;
    file.setFileName(filename);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();
    file.close();
    QJsonDocument d = QJsonDocument::fromJson(val.toUtf8());

    QJsonObject object = d.object();

    if(object.contains("Signals")){
        QJsonArray imported_signals = object["Signals"].toArray();
        QVector<struct Signal> signal_vector;
        struct Signal signal_element;

        for(int i=0; i<imported_signals.count(); i++){
            QJsonObject signal = imported_signals[i].toObject();
            signal_element.datatype = signal["Datatype"].toString();
            signal_element.index = signal["Index"].toInt();
            signal_element.offset = signal["Offset"].toInt();
            signal_element.name = signal["Signalname"].toString();
            signal_vector.append(signal_element);
        }
        setSignals(&signal_vector);

        emit signalsChanged();
    }


}

int Signals::importXLSX(QString filename){

    QVector<struct Signal> new_signals;

    xlnt::workbook wb;
    wb.load(filename.toStdString());
    auto ws = wb.active_sheet();

    std::vector< std::vector<std::string> > theWholeSpreadSheet;
     for (auto row : ws.rows(false)){
        std::vector<std::string> aSingleRow;
         for (auto cell : row){
             aSingleRow.push_back(cell.to_string());
         }
         theWholeSpreadSheet.push_back(aSingleRow);
     }

     unsigned long description_row = -1;
     for (unsigned long i=0; i < theWholeSpreadSheet.size(); i++){
         auto row = theWholeSpreadSheet.at(i);
         if(row[0].compare("Index")==0){
            description_row = i;
         }
     }

     if(description_row <0){
        return -1;
     }

     // Find columns where
     long datatype_column = -1;
     long index_column = 0;
     long name_column = -1;
     long size_column = -1;
     for(unsigned long column=0; column < theWholeSpreadSheet.at(description_row).size(); column++){
         std::string column_value = theWholeSpreadSheet.at(description_row).at(column);
         if(column_value.compare("Datatype")== 0){
             datatype_column = column;
             continue;
         }

         if(column_value.compare("Signalname")==0){
             name_column = column;
             continue;
         }

         if(column_value.compare("Size [Bytes]")==0){
             size_column = column;
             continue;
         }
     }

     if(datatype_column <0 || index_column < 0 || name_column < 0|| size_column<0){
         return -2;
     }

     int offset = 0;

     for(unsigned long i=description_row+1; i< theWholeSpreadSheet.size(); i++){
         struct Signal signal;
         signal.datatype = QString::fromStdString(theWholeSpreadSheet.at(i).at(datatype_column));
         signal.name = QString::fromStdString(theWholeSpreadSheet.at(i).at(name_column));
         signal.index = std::stoi(theWholeSpreadSheet.at(i).at(index_column));
         signal.offset = offset;
         offset+= std::stoi(theWholeSpreadSheet.at(i).at(size_column));
         new_signals.append(signal);
     }

    m_signals.clear();
    m_signals = new_signals;
    emit signalsChanged();
    return 0;
}

void Signals::importSignals(){
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        tr("Open Signals file"), "/home", tr("UDP Logger Config Files (*.udpLoggerSignals, *.xlsx)"));
    if(fileName.split(".").last() == "xlsx"){
        importXLSX(fileName);
        return;
    }
    importJSonFile(fileName);
}
