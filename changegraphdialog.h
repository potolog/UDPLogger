#ifndef CHANGEGRAPHDIALOG_H
#define CHANGEGRAPHDIALOG_H

#include <QDialog>
#include "signals.h"

class Plot;

struct SettingsGraph{
    QColor color;
    int linestyle;
    int scatterstyle;
    struct Signal signal;
    QString name;

};

namespace Ui {
class changeGraphDialog;
}

class changeGraphDialog : public QDialog
{
    Q_OBJECT

public:
    explicit changeGraphDialog(Plot* parent_plot, QWidget* parent, Signals *signal);
    void saveOldSettings();
    struct SettingsGraph getSettings(int index){return m_settings[index];}
    void updateSignals();

    ~changeGraphDialog();
public slots:
    void addElement(struct SettingsGraph* settings_import);
private slots:
    void apply();
    void ok();
    void cancel();
    void deleteElement();
    void addElement();
    void updateData(const QString &text);
    void updateData2();
    void listWidgetRowChanged(int row);
private:
    Ui::changeGraphDialog *ui;
    Plot* m_parent;
    QVector<struct SettingsGraph> m_settings; // old settings
    QVector<struct SettingsGraph> m_settings_new;
    int m_previous_row;

    Signals* m_signals;

};

#endif // CHANGEGRAPHDIALOG_H
