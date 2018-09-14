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


Signals::Signals()
{
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

void Signals::importSignals(){
    QString fileName = QFileDialog::getOpenFileName(nullptr,
        tr("Open Signals file"), "/home", tr("UDP Logger Config Files (*.udpLoggerSignals)"));

    QFile file;
    file.setFileName(fileName);
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
