#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QHostAddress>

class Plots;

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(Plots *parent = nullptr);
    void setSettings(QString project_name, QHostAddress hostname, int buffersize_udp, int buffersize_plot, int port);
    ~SettingsDialog();
private slots:
    void comboHostnameIndexChanged(int index);
    void accepted();
signals:
    void settingsAccepted(QString project_name, QHostAddress hostname, int buffersize_udp, int buffersize_plot, int port);

private:
    Ui::SettingsDialog *ui;
};

#endif // SETTINGSDIALOG_H
