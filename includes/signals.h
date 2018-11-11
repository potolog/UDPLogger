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

#ifndef SIGNALS_H
#define SIGNALS_H
#include <QString>
#include <QVector>
#include <QObject>

struct Signal{
    QString name;
    QString datatype;
    QString unit;
    int offset; // in receive Byte
    int index;
    QString struct_name;
};

struct input_arguments{
    QString datatype;
    QString variable_name;
};

// eventuell erweitern, dass signalnamen von datei eingelesen werden können.
class Signals: public QObject
{
    Q_OBJECT
public:
    Signals();
    int getSignalCount(){return m_signals.length();}
    struct Signal getSignal(int index){return m_signals[index];}
    void setSignals(QVector<struct Signal>* imported_signals){m_signals = *imported_signals; emit signalsChanged();}
    void importJSonFile(QString filename);
    int importXLSX(QString filename);
    bool isStruct(QString variablename);
    QString validateDatatypes(QString datatype, bool &success);
    void parseJsonObject(QJsonObject &object);
    void writeToJsonObject(QJsonObject &object);
    int calculateMinBufferLength();
    QString ifConditionBuffersize(input_arguments buffer_length_variable);
    int calculateDatatypeSize(QString datatype);
private:
    bool ifStructNameExist(const QVector<struct input_arguments>& arguments, QString struct_name, bool& ifstruct);
    void getInputArguments(QVector<struct input_arguments>& arguments);
    void createMemcpyStrings(QVector<QString> &memcpy_strings);

public slots:
    void importSignals();
    void exportUDPFunction();
    void changeSignalSettings(QString relative_header_path, QString additional_includes);
    bool signalExist(struct Signal signal_to_match) const;

signals:
    void signalsChanged();
    void showMessageBox(QString title, QString text);

private:
   QVector<struct Signal> m_signals;
   QString m_header_path;
   QString m_additional_includes;
   QString m_c_path;
};

#endif // SIGNALS_H
