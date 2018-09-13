#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "plots.h"


SettingsDialog::SettingsDialog(Plots *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->spinbox_plot_buffer->setRange(1,2147483647);
    ui->spinbox_udp_buffer->setRange(1,2147483647);
    ui->spinbox_port->setRange(0,65535);
    ui->txt_hostaddress->setInputMask("000.000.000.000;_");

    ui->combo_hostname->addItem("address",QHostAddress::Null);
    ui->combo_hostname->addItem("AnyIPv4",QHostAddress::AnyIPv4);
    ui->combo_hostname->addItem("LocalHost",QHostAddress::LocalHost);
    ui->combo_hostname->addItem("Broadcast",QHostAddress::Broadcast);


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accepted);
    connect(this, &SettingsDialog::settingsAccepted, parent, &Plots::settingsAccepted);
    connect(ui->combo_hostname, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingsDialog::comboHostnameIndexChanged);
}

void SettingsDialog::setSettings(QString project_name, QHostAddress hostname, int buffersize_udp, int buffersize_plot, int port){
    ui->spinbox_plot_buffer->setValue(static_cast<double>(buffersize_plot));
    ui->spinbox_udp_buffer->setValue(static_cast<double>(buffersize_udp));
    ui->spinbox_port->setValue(port);
    ui->combo_hostname->setCurrentIndex(0);
    ui->txt_hostaddress->setText(hostname.toString());

}

void SettingsDialog::comboHostnameIndexChanged(int index){
    int address = ui->combo_hostname->itemData(index).toInt();

    if(address == QHostAddress::AnyIPv4){
        ui->txt_hostaddress->setText("0.0.0.0");
    }else if (address == QHostAddress::Broadcast){
        ui->txt_hostaddress->setText("255.255.255.255");
    }else if (address == QHostAddress::LocalHost){
        ui->txt_hostaddress->setText("127.0.0.1");
    }
}

void SettingsDialog::accepted(){

    int buffersize_udp, buffersize_plot,port;

    buffersize_udp = static_cast<int>(ui->spinbox_udp_buffer->value());
    buffersize_plot = static_cast<int>(ui->spinbox_plot_buffer->value());
    port = static_cast<int>(ui->spinbox_port->value());

    QHostAddress hostname(ui->txt_hostaddress->text());
    QString project_name = ui->txt_project_name->text();


    emit settingsAccepted(project_name, hostname, buffersize_udp, buffersize_plot,port);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
