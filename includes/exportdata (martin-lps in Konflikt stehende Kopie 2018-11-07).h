#ifndef EXPORTDATA_H
#define EXPORTDATA_H

#include <QThread>
#include "udp.h"
class Signals;

class ExportData : public QThread
{
    Q_OBJECT
public:
    explicit ExportData(QString path,QString project_name, QVector<struct udp_message_puffer> &data,int index_first, int index_last, int  udp_buffer_size, Signals* signal, QObject *parent = nullptr);
    void run() override;
private:
    QVector<struct udp_message_puffer> m_data;
    QString m_path;
    int m_last, m_first, m_udp_buffer_size;
    Signals* m_signals;
    QString m_project_name;

Q_SIGNALS:
    void resultReady(const QString &s);

public Q_SLOTS:
};

#endif // EXPORTDATA_H
