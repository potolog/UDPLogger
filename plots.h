#ifndef PLOT_H
#define PLOT_H
#include "qcustomplot.h"
#include <QMouseEvent>
#include "signals.h"
#include <array>
#include <QHostAddress>

class PlotsContextMenu;
class Plots;
class changeGraph;
class UDP;
class QMutex;
class QJsonObject;
class SettingsDialog;

class Plot : public QCustomPlot{
    Q_OBJECT
public:
    Plot(Plots* plots, QWidget* parent, int buffersize_plot, int buffersize_udp, int index, Signals* signal);
    void mousePressEvent(QMouseEvent *ev);
    void addGraphToPlot(struct SettingsGraph* settings);
    bool ifNameExists(QString name);
    QVector<struct Signal>* getSignalSettings(){return &m_signal_settings;}
    void appendYData(){
        m_y_data.resize(m_y_data.size()+1);
        m_y_data[m_y_data.size()-1].resize(m_udp_buffersize);
    }

    void removeData(int index){m_y_data.remove(index);}
    void writeJSON(QJsonObject &object);


private:
    int m_index;
    Plots* m_parent;
    PlotsContextMenu* m_context_menu;
    QMenu* m_menu;
    changeGraph* m_changegpraph_dialog;
    QVector<struct Signal> m_signal_settings;
    double m_ymin;
    double m_ymax;
    double m_xmax;
    double m_xmin;    

    int m_plot_buffer_index;
    int m_plot_buffersize;
    int m_udp_buffersize;
    int buffer_index;
    QVector<QVector<double>> m_y_data;
    QVector<double> m_x_data;

private slots:
    void ShowContextMenu(const QPoint& pos);
    void deletePlot();
    void clearPlot();
    void changeGraphStyle();
public slots:
    void newData(unsigned long index);
    void resizePlotBuffer(int udp_buffersize, int plot_buffersize){
        m_plot_buffersize = plot_buffersize;
        m_udp_buffersize = udp_buffersize;
        for (int i=0; i<m_y_data.size(); i++){
            m_y_data[i].resize(m_plot_buffersize);
        }
        m_x_data.resize(m_plot_buffersize);
    }


signals:
    void deletePlot2(int index);

};

class Plots: public QWidget
{
    Q_OBJECT

public:
    Plots(QWidget *parent, Signals* signal);
    QVBoxLayout* getLayout(){return m_layout;}
    Plot* createNewPlot();
    double* getBufferData(){return m_buffer.data()->data();}
    double getBufferData(int row, int column){return m_buffer[row][column];}
    unsigned long getbufferIndex(){return m_index_buffer;}
    int getBufferCount(){return m_buffersize_udp;}
    void changeDataBufferSize(int size, int size_udp);
    ~Plots();
public slots:
    void deletePlot(int index);
    void newData();
    void exportSettings();
    void importSettings();
    void settings();
    void startUDP();
    void stopUDP();
    void settingsAccepted(QString project_name, QHostAddress hostname, int buffersize_udp, int buffersize_plot, int port);
signals:
    void startUDPReadData();
    void newData2(unsigned long m_index_buffer);
    void resizePlotBuffer(int udp_buffersize, int plot_buffersize);

private:
    QWidget* m_parent;
    QVector<Plot*> m_plots;
    QVBoxLayout* m_layout;
    Signals* m_signals;
    QThread m_udp_thread;
    UDP* m_udp;
    QMutex* m_mutex;
    QHostAddress m_hostaddress;
    int m_port;
    int m_buffersize_udp;
    int m_buffersize_plot;


    QString m_program_version;
    QString m_project_name;
    QString m_fileName;
    bool m_ifudpLogging;
    SettingsDialog* m_settings_dialog;


    unsigned long m_index_buffer;
    int m_buffersize_datas;
    QVector<QVector<double>> m_buffer;

};

#endif // PLOT_H
