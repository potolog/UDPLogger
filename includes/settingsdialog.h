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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QHostAddress>

class Plots;
class QJsonObject;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(Plots *parent = nullptr);
    void setSettings(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int port, int refresh_rate, QString export_filename);
    void createJSONObject(QJsonObject& object);
    void readJSONObject(QJsonObject& object, QString project_name);
    void initSettings();
    ~SettingsDialog();
	void checkSettingsOK();
private slots:
    void comboHostnameIndexChanged(int index);
    void accepted();
	void leSourcePathTextChanged(const QString& text);
	void on_txt_export_path_textChanged(const QString &text);
    void on_btn_browse_export_file_clicked();
	void on_pbBrowseSourcePath_clicked();
	void leHeaderPathTextChanged(const QString& text);

signals:
    void settingsAccepted(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize,
						  int port, int refresh_rate, int skip_element,QString export_filename, QString sourcePath,
                          QString relative_header_path, QString additional_settings);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
