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
    int offset; // in receive Byte
    int index;
};

// eventuell erweitern, dass signalnamen von datei eingelesen werden können.
class Signals: public QObject
{
    Q_OBJECT
public:
    Signals();
    int getSignalCount(){return m_signals.length();}
    struct Signal getSignal(int index){return m_signals[index];}
    void setSignals(QVector<struct Signal>* imported_signals){m_signals = *imported_signals;}

public slots:
    void importSignals();

signals:
    void signalsChanged();

private:
   QVector<struct Signal> m_signals;

};

#endif // SIGNALS_H
