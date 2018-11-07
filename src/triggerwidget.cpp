#include "triggerwidget.h"
#include "ui_triggerwidget.h"

TriggerWidget::TriggerWidget(Signals *signal, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TriggerWidget),
    m_signals(signal)
{
    ui->setupUi(this);

    for(int i=0; i<TriggerType::COUNT_TRIGGER_TYPES; i++){
        ui->cb_triggertype->addItem(trigger_types[i]);
    }

    ui->cb_triggertype->setCurrentIndex(0);
    m_signal_index = 0;
    m_t_after_trigger =0;
    m_t_before_trigger = 0;
    m_trigger_type = TriggerType::RISING_EDGE;
    m_automatic_restart = false;
    m_trigger_level = 0;
    m_trigger_enabled = false;
    ui->checkbox_enable_trigger->setEnabled(false);

}

TriggerWidget::~TriggerWidget()
{
    delete ui;
}

void TriggerWidget::updateSignals(){
    ui->cb_signals->clear();
    struct Signal signal_temp;
    for (int i=0; i< m_signals->getSignalCount(); i++){
        signal_temp = m_signals->getSignal(i);
        ui->cb_signals->addItem(signal_temp.name,i);
    }
    if(m_signals->getSignalCount()>0){
        ui->checkbox_enable_trigger->setEnabled(true);
    }
}

struct Signal TriggerWidget::getTriggerSignal(){
    return m_signals->getSignal(m_signal_index);
}

double TriggerWidget::getTriggerLevel(){
    return m_trigger_level;
}

TriggerType TriggerWidget::getTriggerType(){
    return m_trigger_type;
}

bool TriggerWidget::isTriggerEnabled(){
    return m_trigger_enabled;
}

bool TriggerWidget::ifRestartTrigger(){
    return m_automatic_restart;
}

double TriggerWidget::getTimeAfterTrigger(){
    return m_t_after_trigger;
}

double TriggerWidget::getTimeBeforeTrigger(){
    return m_t_before_trigger;
}

void TriggerWidget::on_spinbox_trigger_level_valueChanged(double arg1)
{
    m_trigger_level = arg1;
}

void TriggerWidget::on_cb_triggertype_activated(int index)
{
    m_trigger_type = static_cast<TriggerType>(index);
}

void TriggerWidget::on_cb_signals_activated(int index)
{
    m_signal_index = index;
}

void TriggerWidget::on_checkbox_enable_trigger_toggled(bool checked)
{
    m_trigger_enabled = checked;
}

void TriggerWidget::on_checkbox_restart_trigger_toggled(bool checked)
{
    m_automatic_restart = checked;
}

void TriggerWidget::on_spinbox_t_before_trigger_valueChanged(double arg1)
{
    m_t_before_trigger = arg1;
}

void TriggerWidget::on_spinbox_t_after_trigger_valueChanged(double arg1)
{
    m_t_after_trigger = arg1;
}

void TriggerWidget::triggered(){
    if(!m_automatic_restart){
        ui->checkbox_enable_trigger->setChecked(false);
        return;
    }
    ui->checkbox_enable_trigger->setChecked(true);
}

void TriggerWidget::disableTrigger(){
    m_trigger_enabled = false;
    m_automatic_restart = false;
    ui->checkbox_restart_trigger->setChecked(false);
    ui->checkbox_enable_trigger->setChecked(false);
}
