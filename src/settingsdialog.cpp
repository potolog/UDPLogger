/***
 *  This file is part of UDPLogger
 *
 *  Copyright (C) 2018 Martin Marmsoler, martin.marmsoler at gmail.com
 *
 *  UDPLogger is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with UDPLogger.  If not, see <http://www.gnu.org/licenses/>.
 ***/

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
    ui->spinbox_refresh_rate->setRange(1,300);
    ui->txt_hostaddress->setInputMask("000.000.000.000;_");
    ui->spinbox_use_element_count->setRange(1,2147483647);

    ui->combo_hostname->addItem("address",QHostAddress::Null);
    ui->combo_hostname->addItem("AnyIPv4",QHostAddress::AnyIPv4);
    ui->combo_hostname->addItem("LocalHost",QHostAddress::LocalHost);
    ui->combo_hostname->addItem("Broadcast",QHostAddress::Broadcast);
    ui->txt_relative_header_path->setText("");


    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::accepted);
    connect(this, &SettingsDialog::settingsAccepted, parent, &Plots::settingsAccepted);
    connect(ui->combo_hostname, qOverload<int>(&QComboBox::currentIndexChanged), this, &SettingsDialog::comboHostnameIndexChanged);
	connect(ui->leExportSourceFilePath, &QLineEdit::textChanged, this, &SettingsDialog::leSourcePathTextChanged);
	connect(ui->txt_relative_header_path, &QLineEdit::textChanged, this, &SettingsDialog::leHeaderPathTextChanged);

    initSettings();
    accepted(); // initially setting settings
}

void SettingsDialog::setSettings(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int port,int refresh_rate, QString export_filename){
    ui->spinbox_plot_buffer->setValue(static_cast<double>(plot_buffersize));
    ui->spinbox_udp_buffer->setValue(static_cast<double>(udp_buffersize));
    ui->spinbox_port->setValue(port);
    ui->combo_hostname->setCurrentIndex(0);
    ui->txt_hostaddress->setText(hostname.toString());
    ui->txt_project_name->setText(project_name);
    ui->txt_export_path->setText(export_filename);
    ui->spinbox_refresh_rate->setValue(refresh_rate);
}

void SettingsDialog::initSettings(){
    QString project_name = "Project1";
    QHostAddress hostaddress = QHostAddress::Any;
    int udp_buffersize = 400;
    int plot_buffersize = 200;
    int port = 60000;
    int refresh_rate = 60;
    setSettings(project_name, hostaddress, udp_buffersize, plot_buffersize, port,refresh_rate, "");
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

    int udp_buffersize, plot_buffersize, port;
    int refresh_rate;
    bool export_data;
    int use_data_count;
	QString sourcePath;
    QString relative_header_path;
    QString additional_includes;

    udp_buffersize = static_cast<int>(ui->spinbox_udp_buffer->value());
    plot_buffersize = static_cast<int>(ui->spinbox_plot_buffer->value());
    refresh_rate = static_cast<int>(ui->spinbox_refresh_rate->value());
    port = static_cast<int>(ui->spinbox_port->value());
    use_data_count = static_cast<int>(ui->spinbox_use_element_count->value());
    relative_header_path = ui->txt_relative_header_path->text();

    QHostAddress hostname(ui->txt_hostaddress->text());
    QString project_name = ui->txt_project_name->text();
    QString export_filename = ui->txt_export_path->text();
    additional_includes = ui->txt_additional_includes->text();
	sourcePath = ui->leExportSourceFilePath->text();

	emit settingsAccepted(project_name, hostname, udp_buffersize, plot_buffersize, port,refresh_rate, use_data_count, export_filename, sourcePath, relative_header_path, additional_includes);
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_btn_browse_export_file_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,
            tr("Set export file path"), "/home");
    ui->txt_export_path->setText(path);
}

void SettingsDialog::on_pbBrowseSourcePath_clicked() {
	QString path = QFileDialog::getExistingDirectory(this,
			tr("Set export file path"), "/home");
	ui->leExportSourceFilePath->setText(path);
}

void SettingsDialog::createJSONObject(QJsonObject& object){
    QJsonObject network_settings;
    network_settings["HostAddress"] = QHostAddress(ui->txt_hostaddress->text()).toString();
    network_settings["Port"] = static_cast<int>(ui->spinbox_port->value());
    network_settings["UDPPufferSize"] = static_cast<int>(ui->spinbox_udp_buffer->value());
    object["NetworkSettings"] = network_settings;
    object["PlotPufferSize"] = static_cast<int>(ui->spinbox_plot_buffer->value());
    object["ExportDataFile"] =ui->txt_export_path->text();
    object["RefreshRate"] = static_cast<int>(ui->spinbox_refresh_rate->value());
    object["SkipElement"] = static_cast<int>(ui->spinbox_use_element_count->value());
	object["ExportSourceFile"] = ui->leExportSourceFilePath->text();
    object["relative_header_path"] = ui->txt_relative_header_path->text();
    object["additionalIncludes"] = ui->txt_additional_includes->text();
}

void SettingsDialog::readJSONObject(QJsonObject& object, QString project_name){

    ui->spinbox_plot_buffer->setValue(static_cast<double>(object["PlotPufferSize"].toInt()));
    ui->spinbox_refresh_rate->setValue(static_cast<double>(object["RefreshRate"].toInt()));
    ui->spinbox_use_element_count->setValue(static_cast<double>(object["SkipElement"].toInt()));
	ui->leExportSourceFilePath->setText(object["ExportSourceFile"].toString());
    ui->txt_relative_header_path->setText(object["relative_header_path"].toString());

    QJsonObject network_settings = object["NetworkSettings"].toObject();
    ui->txt_hostaddress->setText(QHostAddress(network_settings["HostAddress"].toString()).toString());
    ui->spinbox_port->setValue(static_cast<double>(network_settings["Port"].toInt()));
    ui->spinbox_udp_buffer->setValue(static_cast<double>(network_settings["UDPPufferSize"].toInt()));

    ui->txt_additional_includes->setText(object["additionalIncludes"].toString());
    ui->txt_project_name->setText(project_name);
    ui->txt_export_path->setText(object["ExportDataFile"].toString());

    emit accepted();

}

void SettingsDialog::on_txt_export_path_textChanged(const QString &text) {
	Q_UNUSED(text)
	checkSettingsOK();
}

void SettingsDialog::leSourcePathTextChanged(const QString &text) {
	Q_UNUSED(text)
	checkSettingsOK();
}

void SettingsDialog::leHeaderPathTextChanged(const QString& text) {
	Q_UNUSED(text)
	checkSettingsOK();
}

void SettingsDialog::checkSettingsOK() {

	if (!QDir(ui->txt_export_path->text()).exists()) {
		ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
		ui->buttonBox->setToolTip("Data export path is not valid");
		ui->txt_export_path->setStyleSheet("QLineEdit{background:red;}");
		return;
	} else {
		ui->txt_export_path->setStyleSheet(QString());
	}

	if (!QDir(ui->leExportSourceFilePath->text()).exists()) {
		ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
		ui->buttonBox->setToolTip("ExportSourceFile path is not valid");
		ui->leExportSourceFilePath->setStyleSheet("QLineEdit{background:red;}");
		return;
	} else {
		ui->leExportSourceFilePath->setStyleSheet(QString());
	}

	if (!QDir(ui->leExportSourceFilePath->text()+"/"+ui->txt_relative_header_path->text()).exists()) {
		ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
		ui->buttonBox->setToolTip("Relative header path does not exist");
		ui->txt_relative_header_path->setStyleSheet("QLineEdit{background:red;}");
		return;
	} else {
		ui->txt_relative_header_path->setStyleSheet(QString());
	}
	ui->buttonBox->setToolTip("Acccept settings");
	ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
}
