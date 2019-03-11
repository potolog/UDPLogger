#ifndef CHANGEDIAGRAMPROPERTIES_H
#define CHANGEDIAGRAMPROPERTIES_H

#include <QDialog>

namespace Ui {
class ChangeDiagramProperties;
}

class ChangeDiagramProperties : public QDialog
{
	Q_OBJECT

public:
	explicit ChangeDiagramProperties(QWidget *parent = nullptr);
	~ChangeDiagramProperties();
	void setProperties(int heigth);
private slots:
	void sbHeightChanged(int height);
signals:
	void heightChanged(int height);


private:
	Ui::ChangeDiagramProperties *ui;
	bool m_suppress{false};
};

#endif // CHANGEDIAGRAMPROPERTIES_H
