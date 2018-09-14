#ifndef UDP_H
#define UDP_H

#include <array>
#include <QObject>
#include <QVector>

class QFile;
class QUdpSocket;
class QMutex;
class Plots;
class QHostAddress;

class UDP: public QObject
{
    Q_OBJECT

public:
    UDP(Plots* parent, QMutex* mutex);
    bool init(QHostAddress hostaddress, quint16 port, int buffer_size, bool export_data, QString filename);
    bool init();
    ~UDP();
public slots:
    void readData();
    float* getFloatPointer(int position){return (float*)(&m_data.data()[position]);}
    double* getDoublePointer(int position){return (double*)(&m_data.data()[position]);}
    char* getCharPointer(int position){return &m_data.data()[position];}
    int* getIntPointer(int position){return (int*)(&m_data.data()[position]);}
    void connectDataReady();
    void disconnectDataReady();

signals:
    void newData();

private:
    int m_actual_index;
    QUdpSocket *m_socket;
    QVector<char> m_data;
    QVector<char> m_data_temp;
    const int m_max_data = 400;

    bool m_export_data;
    QFile* file;
    bool m_if_file_ready;

    QMutex* m_mutex;
    unsigned long m_index_read; // index which data was read
    Plots* m_parent;



};

#endif // UDP_H
