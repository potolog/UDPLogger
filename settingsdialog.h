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
    void setSettings(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int data_buffersize, int port, bool export_data, QString export_filename);
    void createJSONObject(QJsonObject& object);
    void readJSONObject(QJsonObject& object, QString project_name);
    ~SettingsDialog();
private slots:
    void comboHostnameIndexChanged(int index);
    void accepted();
    void on_txt_export_path_textChanged(const QString &arg1);

    void on_btn_browse_export_file_clicked();

signals:
    void settingsAccepted(QString project_name, QHostAddress hostname, int udp_buffersize, int plot_buffersize, int data_buffersize, int port, bool export_data, QString export_filename);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
