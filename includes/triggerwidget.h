#ifndef TRIGGERWIDGET_H
#define TRIGGERWIDGET_H

#include <QWidget>
#include "signals.h"

namespace Ui {
class TriggerWidget;
}
class Signals;

enum TriggerType{
    RISING_EDGE,
    FALLING_EDGE,
    ALL_EDGES,
    COUNT_TRIGGER_TYPES // all new types must be before this
};

const QString trigger_types[TriggerType::COUNT_TRIGGER_TYPES] = {"RISING EDGE", "FALLING EDGE", "ALL EDGES"};

class TriggerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TriggerWidget(Signals* signal, QWidget *parent = nullptr);
    ~TriggerWidget();
    struct Signal getTriggerSignal();
    double getTriggerLevel();
    TriggerType getTriggerType();
    bool isTriggerEnabled();
    bool ifRestartTrigger();
    double getTimeAfterTrigger();
    double getTimeBeforeTrigger();
    void disableTrigger();
public slots:
    void updateSignals();
    void triggered();

private slots:
    void on_spinbox_trigger_level_valueChanged(double arg1);

    void on_cb_triggertype_activated(int index);

    void on_cb_signals_activated(int index);

    void on_checkbox_enable_trigger_toggled(bool checked);

    void on_checkbox_restart_trigger_toggled(bool checked);

    void on_spinbox_t_before_trigger_valueChanged(double arg1);

    void on_spinbox_t_after_trigger_valueChanged(double arg1);

private:
    Ui::TriggerWidget *ui;
    Signals* m_signals;
    double m_trigger_level;
    TriggerType m_trigger_type;
    int m_signal_index;
    bool m_trigger_enabled;
    bool m_automatic_restart;
    double m_t_before_trigger;
    double m_t_after_trigger;
};

#endif // TRIGGERWIDGET_H
