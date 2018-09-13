#ifndef SIGNALS_H
#define SIGNALS_H
#include <QString>
#include <QVector>

struct Signal{
    QString name;
    QString datatype;
    int offset; // in receive Byte
    int index;
};

// eventuell erweitern, dass signalnamen von datei eingelesen werden können.
class Signals
{
public:
    Signals();
    int getSignalCount(){return m_signals.length();}
    struct Signal getSignal(int index){return m_signals[index];}
    void setSignals(QVector<struct Signal>* imported_signals){m_signals = *imported_signals;}

private:
   QVector<struct Signal> m_signals;

};

#endif // SIGNALS_H
