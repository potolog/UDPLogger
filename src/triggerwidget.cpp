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
    ui->checkbox_enable_trigger->setEnabled(false);
	ui->lbl_status->setStyleSheet("QWidget { background-color: green }");
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
	if(m_signals->getSignalCount()-1 >= m_signal_index){
		return m_signals->getSignal(m_signal_index);
	}
	struct Signal signal;
	return signal;
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
	triggerFinished();
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

void TriggerWidget::newTriggerValue(double value){
	ui->lbl_value->setText(QString::number(value));
}

void TriggerWidget::triggerFinished() {
	ui->lbl_status->setStyleSheet("QWidget { background-color: green }");
	ui->pb_start_trigger->setEnabled(true);
	ui->checkbox_enable_trigger->setEnabled(true);
}

void TriggerWidget::on_pb_start_trigger_clicked()
{
	emit startTrigger();
}

void TriggerWidget::triggerStarted() {
	ui->lbl_status->setStyleSheet("QWidget { background-color: yellow }");
	ui->pb_start_trigger->setEnabled(false);
	ui->checkbox_enable_trigger->setEnabled(false);
}
