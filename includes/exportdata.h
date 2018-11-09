#ifndef EXPORTDATA_H
#define EXPORTDATA_H

#include <QThread>
#include "udp.h"
class Signals;

class ExportData : public QThread
{
    Q_OBJECT
public:
    explicit ExportData(QString path, QString project_name,QVector<struct udp_message_puffer> &data,int index_first, uint64_t index_last, int  udp_buffer_size, Signals* signal, QObject *parent = nullptr);
    void run() override;
private:
    QVector<struct udp_message_puffer> m_data;
    QString m_path;
    QString m_project_name;
    int m_last, m_first, m_udp_buffer_size;
    Signals* m_signals;


signals:
    void resultReady();
    void showInfoMessage(QString title, QString text);

public slots:
};

#endif // EXPORTDATA_H
