#include "changediagramproperties.h"
#include "ui_changediagramproperties.h"

ChangeDiagramProperties::ChangeDiagramProperties(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ChangeDiagramProperties)
{
	ui->setupUi(this);

	connect(ui->sbMinHeight, qOverload<int>(&QSpinBox::valueChanged), this, &ChangeDiagramProperties::sbHeightChanged);
}

void ChangeDiagramProperties::sbHeightChanged(int height) {
	if (m_suppress)
		return;
	emit heightChanged(height);
}

ChangeDiagramProperties::~ChangeDiagramProperties()
{
	delete ui;
}

void ChangeDiagramProperties::setProperties(int heigth) {
	m_suppress = true;
	ui->sbMinHeight->setValue(height());
	m_suppress = false;
}
