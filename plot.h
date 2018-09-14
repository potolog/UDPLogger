#ifndef PLOT_H
#define PLOT_H
#include <QMouseEvent>
#include "signals.h"
#include "qcustomplot.h"

class Plots;
class changeGraphDialog;
class PlotsContextMenu;

class Plot : public QCustomPlot{
    Q_OBJECT
public:
    Plot(Plots* plots, QWidget* parent, int plot_buffersize, int udp_buffersize, int index, Signals* signal);
    void mousePressEvent(QMouseEvent *ev);
    void addGraphToPlot(struct SettingsGraph* settings);
    bool ifNameExists(QString name);
    QVector<struct Signal>* getSignalSettings(){return &m_signal_settings;}
    void appendYData();

    void removeData(int index){m_y_data.remove(index);}
    void writeJSON(QJsonObject &object);

    ~Plot();

private:
    int m_index;
    Plots* m_parent;
    PlotsContextMenu* m_context_menu;
    QMenu* m_menu;
    changeGraphDialog* m_changegpraph_dialog;
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
    void resizePlotBuffer(int udp_buffersize, int plot_buffersize);
    void signalsChanged();


signals:
    void deletePlot2(int index);

};

#endif // PLOT_H
