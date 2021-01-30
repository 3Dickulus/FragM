#include "ExpSpinBox.h"

#include <limits>

ExpSpinBox::ExpSpinBox(QWidget * parent)
    : QDoubleSpinBox(parent)
{
	setDecimals(__DBL_MAX_10_EXP__ + __DBL_DIG__);
	QDoubleSpinBox::setDecimals(__DBL_MAX_10_EXP__ + __DBL_DIG__);
	setLocale(QLocale("C"));
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
    /*
     * The conversion algorithm will try to find the shortest accurate representation for the given number.
     * "Accurate" means that you get the exact same number back from an inverse conversion on the generated string representation.
     * defaults arg( double a, int fieldWidth = 0, char fmt = 'g', int prec = -1, QChar fillChar = QLatin1Char(' ') )
     */ 

    
	// convert to string -> Using exponential or standard display
    return QString("%1").arg(value, 20, sfmt ? 'e' : 'g', QLocale::FloatingPointShortest);
}

double ExpSpinBox::valueFromText(const QString &text) const
{
    return text.toDouble();
}

