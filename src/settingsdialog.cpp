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
    ui->spinbox_data_buffersize->setRange(1,2147483647);
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

void SettingsDialog::setSettings(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int data_buffersize, int port, bool export_data, QString export_filename){
    ui->spinbox_plot_buffer->setValue(static_cast<double>(plot_buffersize));
    ui->spinbox_udp_buffer->setValue(static_cast<double>(udp_buffersize));
    ui->spinbox_data_buffersize->setValue(static_cast<double>(data_buffersize));
    ui->spinbox_port->setValue(port);
    ui->combo_hostname->setCurrentIndex(0);
    ui->txt_hostaddress->setText(hostname.toString());
    ui->txt_project_name->setText(project_name);
    ui->checkbox_export_data->setChecked(export_data);
    ui->txt_export_path->setText(export_filename);

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

    int udp_buffersize, plot_buffersize, data_buffersize, port;
    bool export_data;

    udp_buffersize = static_cast<int>(ui->spinbox_udp_buffer->value());
    plot_buffersize = static_cast<int>(ui->spinbox_plot_buffer->value());
    data_buffersize = static_cast<int>(ui->spinbox_data_buffersize->value());
    port = static_cast<int>(ui->spinbox_port->value());
    export_data = ui->checkbox_export_data->isChecked();

    QHostAddress hostname(ui->txt_hostaddress->text());
    QString project_name = ui->txt_project_name->text();
    QString export_filename = ui->txt_export_path->text();

    emit settingsAccepted(project_name, hostname, udp_buffersize, data_buffersize, plot_buffersize, port, export_data, export_filename);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_txt_export_path_textChanged(const QString &arg1)
{
    if(arg1.compare("")==0){
        ui->checkbox_export_data->setChecked(false);
    }else{
        ui->checkbox_export_data->setChecked(true);
    }
}

void SettingsDialog::on_btn_browse_export_file_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
            tr("Set export file"), "/home",
            tr("UDP Logger Data Files (*.udpLoggerData)"));
    ui->txt_export_path->setText(fileName);
}

void SettingsDialog::createJSONObject(QJsonObject& object){
    QJsonObject network_settings;
    network_settings["HostAddress"] = QHostAddress(ui->txt_hostaddress->text()).toString();
    network_settings["Port"] = static_cast<int>(ui->spinbox_port->value());
    network_settings["UDPPufferSize"] = static_cast<int>(ui->spinbox_udp_buffer->value());
    object["NetworkSettings"] = network_settings;
    object["PlotPufferSize"] = static_cast<int>(ui->spinbox_plot_buffer->value());
    object["DataPufferSize"] = static_cast<int>(ui->spinbox_data_buffersize->value());
    object["ExportData"] = ui->checkbox_export_data->isChecked();
    object["ExportDataFile"] =ui->txt_export_path->text();
}

void SettingsDialog::readJSONObject(QJsonObject& object, QString project_name){

    ui->spinbox_plot_buffer->setValue(static_cast<double>(object["PlotPufferSize"].toInt()));
    ui->spinbox_data_buffersize->setValue(static_cast<double>(object["DataPufferSize"].toInt()));
    ui->checkbox_export_data->setChecked(object["ExportData"].toBool())
            ;
    QJsonObject network_settings = object["NetworkSettings"].toObject();
    ui->txt_hostaddress->setText(QHostAddress(network_settings["HostAddress"].toString()).toString());
    ui->spinbox_port->setValue(static_cast<double>(network_settings["Port"].toInt()));
    ui->spinbox_udp_buffer->setValue(static_cast<double>(network_settings["UDPPufferSize"].toInt()));

    ui->txt_project_name->setText(project_name);
    ui->txt_export_path->setText(object["ExportDataFile"].toString());

    accepted();

}
