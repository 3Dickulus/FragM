#ifndef QDOUBLERANGESLIDER_H
#define QDOUBLERANGESLIDER_H

#include <QWidget>
#include "qrangeslider.h"
#include <limits>

class qDoubleRangeSlider
: public QWidget,
public RangeSliderUnitConverter
{
  Q_OBJECT
public:
  typedef QPair<double,double> range_t;

  static double numericalLimitMax() {
    return std::numeric_limits<float>::max();
  }
  static double numericalLimitMin() {
    return -std::numeric_limits<float>::max();
  }
  static range_t numericalLimits() {
    return range_t(numericalLimitMin(),numericalLimitMax());
  }


  Q_PROPERTY(bool logaritmic READ isLogarithmic WRITE setLogarithmic)
  Q_PROPERTY(range_t range READ range WRITE setRange NOTIFY rangeChanged)
  Q_PROPERTY(range_t cutoffRange
  READ cutoffRange
  WRITE setCutoffRange
  NOTIFY cutoffRangeChanged)

  explicit qDoubleRangeSlider(QWidget* parent = 0);
  explicit qDoubleRangeSlider(Qt::Orientation orientation,
                              QWidget* parent = 0);

  range_t range() const;
  range_t cutoffRange() const;
  bool isLogarithmic() const;
  double tickInterval() const;

public slots:
  void setRange(QPair<double, double>);
  void setCutoffRange(QPair<double, double>);
  void setLogarithmic(bool logaritmic);
  void setTickInterval(double tickInterval);

signals:
  void rangeChanged(QPair<double, double>);
  void cutoffRangeChanged(QPair<double, double>);

private slots:
  void rangeChanged(QPair<int,int>);

private:
  void setup();
  void resetInternalCutoffRange();
  void resetInternalRange();



  qRangeSlider* slider_;

  QPair<double, double> cutoffRange_;
  QPair<double, double> range_;

  QPair<int,int> expectValue_;

  double tickInterval_;

  double convertFromBaseToDouble(int) const;
  QVariant convertFromBase(int) const;
  QPair<double,double> convertFromBase(QPair<int, int>) const;

  int convertToBase(QVariant value) const;
  int convertToBase(double value) const;
  QPair<int, int> convertToBase(QPair<double, double> value) const;

  static bool cmp(int a, int b, uint tolerance = 0);
  static bool cmp(const QPair<int, int>& a,
                  const QPair<int, int>& b,
                  uint tolerance = 0);

  static bool clamp(QPair<double,double>& value,
                    const QPair<double,double>& limits);
  static bool clamp(double& value, const QPair<double,double>& limits);

  bool isLogarithmic_;

  static const uint RANGE_SLIDER_MAX = 2048;
  static const uint RANGE_SLIDER_MIN = 0;
  static const uint RANGE_SLIDER_DIV = RANGE_SLIDER_MAX - RANGE_SLIDER_MIN;
};

#endif // QDOUBLESLIDER_H
