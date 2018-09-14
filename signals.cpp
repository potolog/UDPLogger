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
