#include "ExpSpinBox.h"

#include <limits>

ExpSpinBox::ExpSpinBox(QWidget * parent)
    : QDoubleSpinBox(parent)
{
	setDecimals(1000);
	QDoubleSpinBox::setDecimals(1000);
	
	// set Range to maximum possible values
	double doubleMax = std::numeric_limits<double>::max();
	setRange(-doubleMax, doubleMax);
}

int ExpSpinBox::decimals() const
{
	return dispDecimals;
}

void ExpSpinBox::setDecimals(int value)
{
	dispDecimals = value;
	QDoubleSpinBox::setDecimals(dispDecimals);
}

/*!
 *  text to be displayed in spinbox
 */
QString ExpSpinBox::textFromValue(double value) const
{
	// convert to string -> Using exponential or standard display
    return QString::number(value, sfmt ? 'e' : 'g', dispDecimals);
}

double ExpSpinBox::valueFromText(const QString &text) const
{
    return text.toDouble();
}

