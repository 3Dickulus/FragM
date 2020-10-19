#include "ExpSpinBox.h"

#include <limits>

ExpSpinBox::ExpSpinBox(QWidget * parent)
    : QDoubleSpinBox(parent)
{
	setDecimals(__DBL_MAX_10_EXP__ + __DBL_DIG__);
	QDoubleSpinBox::setDecimals(__DBL_MAX_10_EXP__ + __DBL_DIG__);
	
	// set Range to maximum possible values
	double doubleMax = std::numeric_limits<double>::max();
	setRange(-doubleMax, doubleMax);
	QDoubleSpinBox::setRange(-doubleMax, doubleMax);
    setGroupSeparatorShown(false);
    QDoubleSpinBox::setGroupSeparatorShown(false);
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
    return locale().toString(value, sfmt ? 'e' : 'g', dispDecimals);
}

double ExpSpinBox::valueFromText(const QString &text) const
{
    return text.toDouble();
}

