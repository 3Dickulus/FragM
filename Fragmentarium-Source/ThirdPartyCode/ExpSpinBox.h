#ifndef __ExpSpinBox_H__
#define __ExpSpinBox_H__

#include <QDoubleSpinBox>
#include <QDebug>
#include <QString>

class ExpSpinBox : public QDoubleSpinBox
{
Q_OBJECT
public:
    ExpSpinBox(QWidget * parent = 0);

	int decimals() const;
	void setDecimals(int value);
    QString textFromValue ( double value ) const;
    double valueFromText ( const QString & text ) const;
    void setScientificFormat(bool f=true) {sfmt=f; setValue( value() );};

private:
	int dispDecimals;

private:
    bool sfmt;

public slots:

};

#endif
