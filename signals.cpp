#include "signals.h"

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
