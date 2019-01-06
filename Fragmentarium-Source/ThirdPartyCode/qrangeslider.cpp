/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qrangeslider.h"
#ifndef QT_NO_SLIDER
#ifndef QT_NO_ACCESSIBILITY
#include <QtGui/QAccessible>
#endif
#include <cassert>

#include <cmath>

#include <QtCore/QDebug>
#include <QtCore/QEvent>
#include <QApplication>
#include <QPainter>
#include <QToolTip>
#include <QStyle>
#include <QStyleOption>

class QStyleRangeSlider : public QStyle {
public:
  static const int grooveMarginVertical;
  static const int grooveMarginHorizontal;
  static const int markerWidth;
  static const int tipOffset;
  static const float tickLogBase;

  static const QStyle::PixelMetric PM_RangeSliderLength =
    QStyle::PixelMetric(QStyle::PM_CustomBase + 1);

  void drawPrimitive(QStyle::PrimitiveElement element,
                     const QStyleOption* option,
                     QPainter* painter,
                     const QWidget* widget) const
  {
    realStyle_->drawPrimitive(element, option, painter, widget);
  }


  void drawControl(QStyle::ControlElement element,
                   const QStyleOption* option,
                   QPainter* painter,
                   const QWidget* widget) const
  {
    realStyle_->drawControl(element, option, painter, widget);
  }


  QRect subElementRect(QStyle::SubElement element,
                       const QStyleOption* option,
                       const QWidget* widget) const
  {
    return realStyle_->subElementRect(element, option, widget);
  }


  void drawComplexControl(QStyle::ComplexControl control,
                          const QStyleOptionComplex* option,
                          QPainter* painter,
                          const QWidget* widget) const
  {
    if (control == QStyle::CC_CustomBase) {
      const qRangeSlider* rSlider = static_cast<const qRangeSlider*>(widget);
      QRect bbox = rSlider->getBBox();
      QPair<int, int> range = rSlider->range();
      QPair<int, int> cutoffRange = rSlider->cutoffRange();
      paintGroove(*painter, bbox);

      paintFilling(*painter, bbox, range, cutoffRange);
//       paintTicks(*painter, bbox, cutoffRange,
//                  rSlider->tickInterval() /*,rSlider->isLogarithmic()*/ );
      paintMarker(*painter, bbox, range, cutoffRange, FIRST);
      paintMarker(*painter, bbox, range, cutoffRange, SECOND);
      return;
    }

    return realStyle_->drawComplexControl(control, option, painter, widget);
  }

  QStyle::SubControl hitTestComplexControl(QStyle::ComplexControl control,
                                           const QStyleOptionComplex* option,
                                           const QPoint& position,
                                           const QWidget* widget) const
  {
    if (control == QStyle::CC_CustomBase) {
      QRect bbox = static_cast<const qRangeSlider*>(widget)->getBBox();
      const qRangeSlider* rSlider = static_cast<const qRangeSlider*>(widget);
      QPolygon p = getMarkerArea(bbox, rSlider->range(),
                                 rSlider->cutoffRange(), FIRST);
      if (p.containsPoint(position, Qt::OddEvenFill))
        return QStyle::SC_SliderHandle;
      p = getMarkerArea(bbox, rSlider->range(),
                        rSlider->cutoffRange(), SECOND);
      if (p.containsPoint(position, Qt::OddEvenFill))
        return SC_SliderHandle2;
      return QStyle::SC_None;
    }

    return realStyle_->hitTestComplexControl(control,
                                             option,
                                             position,
                                             widget);
  }

  int pixelMetric(QStyle::PixelMetric metric,
                  const QStyleOption* option,
                  const QWidget* widget) const
  {
    switch(metric) {
    default:
      return realStyle_->pixelMetric(metric, option, widget);
    }
  }

  QRect subControlRect(QStyle::ComplexControl control,
                       const QStyleOptionComplex* option,
                       QStyle::SubControl subControl,
                       const QWidget* widget) const
  {
    return realStyle_->subControlRect(control, option, subControl, widget);
  }

  QSize sizeFromContents(QStyle::ContentsType type,
                         const QStyleOption* option,
                         const QSize& contentsSize,
                         const QWidget* widget) const
  {
    return realStyle_->sizeFromContents(type, option, contentsSize, widget);
  }

  int styleHint(QStyle::StyleHint hint,
                const QStyleOption* option,
                const QWidget* widget,
                QStyleHintReturn* returnData) const
  {
    return realStyle_->styleHint(hint, option, widget, returnData);
  }

  QPixmap standardPixmap(QStyle::StandardPixmap standardPixmap,
                         const QStyleOption* option,
                         const QWidget* widget) const
  {
    return realStyle_->standardPixmap(standardPixmap, option, widget);
  }

  int layoutSpacing ( QSizePolicy::ControlType control1,
		      QSizePolicy::ControlType control2,
		      Qt::Orientation orientation,
		      const QStyleOption * option = 0,
		      const QWidget * widget = 0 ) const
  {
    return realStyle_->layoutSpacing ( control1, control2, orientation, option, widget);
  }

  QIcon standardIcon ( StandardPixmap standardIcon,
		       const QStyleOption * option = 0,
		       const QWidget * widget = 0 ) const
  {
    return realStyle_->standardIcon ( standardIcon, option, widget );
  }

  QPixmap generatedIconPixmap(QIcon::Mode iconMode,
                              const QPixmap& pixmap,
                              const QStyleOption* option) const
  {
    return realStyle_->generatedIconPixmap(iconMode, pixmap, option);
  }


  void setRealStyle(QStyle* realStyle)
  {
    realStyle_ = realStyle;
  }
  enum WHICH { NONE = 0,
               FIRST = 0x1,
               SECOND = 0x2,
               BOTH = FIRST | SECOND };

  const static QStyle::SubControl SC_SliderHandle2 =
    QStyle::SubControl(0x00000008);

  void paintGroove(QPainter& p, const QRect& bbox) const;
  void paintFilling(QPainter& p, const QRect& bbox,
                    const QPair<int, int>& range,
                    const QPair<int, int>& cutoffRange) const;
  void paintMarker(QPainter& p,
                   const QRect& bbox,
                   const QPair<int, int>& range,
                   const QPair<int, int>& cutoffRange,
                   WHICH which) const;


  QPolygon getMarkerArea(const QRect& bbox,
                         const QPair<int, int>& range,
                         const QPair<int, int>& cutoffRange,
                         WHICH which) const;
  void paintTicks(QPainter& p, const QRect& bbox,
                  const QPair<int, int>& cutoffRange,
                  float tickInterval /*,bool logarithmic*/ ) const;

  int getGrooveX(const QRect& bbox) const {
    //    return int(bbox.x()+factor*.5*bbox.width());
    return int(bbox.x() + grooveMarginHorizontal);
  }
  int getGrooveY(const QRect& bbox) const {
    return int(bbox.y() + grooveMarginVertical);
  }
  int getGrooveWidth(const QRect& bbox) const {
    //    return int((1.0-factor)*bbox.width());
    return int(bbox.width() - grooveMarginHorizontal * 2);
  }
  int getGrooveHeight(const QRect& bbox) const {
    return int(bbox.height() - 2 * grooveMarginVertical);
  }
  int getPosMin(const QRect& bbox, int min,
                const QPair<int, int>& cutoffRange) const {
    if (cutoffRange.second == cutoffRange.first)
      return getGrooveX(bbox);
    return int(getGrooveX(bbox) + getGrooveWidth(bbox) *
               (min - cutoffRange.first)/
               (1.0 * (cutoffRange.second - cutoffRange.first)));
  }
  int getPosMax(const QRect& bbox,
                int max,
                const QPair<int, int>& cutoffRange) const {
    return getPosMin(bbox, max, cutoffRange);
    //return int(getGrooveX(bbox)+0.01*max*getGrooveWidth(bbox));
  }

//   void ppoint(const QPoint& p) {
    //std::cout << "(" << p.x() << "," << p.y() << ")";
//   }


  qRangeSlider::ELEMENT getElement(const QRect& bbox,
                                   const QPair<int, int>& range,
                                   const QPair<int, int>& cutoffRange,
                                   const QPoint& pos)
  {
    QPolygon p = getMarkerArea(bbox, range, cutoffRange, FIRST);
    if (p.containsPoint(pos, Qt::OddEvenFill))
      return qRangeSlider::FIRST;
    p = getMarkerArea(bbox, range, cutoffRange, SECOND);
    if (p.containsPoint(pos, Qt::OddEvenFill))
      return qRangeSlider::SECOND;

    return qRangeSlider::NONE;
  }

private:
  QStyle* realStyle_;
};


const int QStyleRangeSlider::tipOffset = 4;
const int QStyleRangeSlider::markerWidth = 6;
const int QStyleRangeSlider::grooveMarginVertical = tipOffset + 0;
const int QStyleRangeSlider::grooveMarginHorizontal = markerWidth + 3;
const float QStyleRangeSlider::tickLogBase = 1;
QStyleRangeSlider* qRangeSlider::styleRangeSlider_ = 0;


/*!
  Initialize \a option with the values from this qRangeSlider. This
  method is useful for subclasses when they need a
  QStyleOptionSlider, but don't want to fill in all the information
  themselves.

  \sa QStyleOption::initFrom()
*/


void qRangeSlider::initStyleOption(QStyleOptionRangeSlider* option)
{

  if (!option)
    return;

  //styleOptionRangeSlider_ = QSharedPointer<QStyleOptionRangeSlider>(option);
  //    Q_D(const qRangeSlider);
  /*
  option->initFrom(this);
  //option->subControls = QStyle::SC_None;
  option->activeSubControls = QStyle::SC_None;
  */
}


/*!
  \class qRangeSlider
  \brief The qRangeSlider widget provides a vertical or horizontal slider.

  \ingroup basicwidgets


  The slider is the classic widget for controlling a bounded value.
  It lets the user move a slider handle along a horizontal or vertical
  groove and translates the handle's position into an integer value
  within the legal range.

  qRangeSlider has very few of its own functions; most of the
  functionality is in QAbstractSlider. The most useful functions are
  setValue() to set the slider directly to some value;
  triggerAction() to simulate the effects of clicking (useful for
  shortcut keys); setSingleStep(), setPageStep() to set the steps;
  and setMinimum() and setMaximum() to define the range of the
  scroll bar.

  qRangeSlider provides methods for controlling tickmarks.  You can use
  setTickPosition() to indicate where you want the tickmarks to be,
  setTickInterval() to indicate how many of them you want. the
  currently set tick position and interval can be queried using the
  tickPosition() and tickInterval() functions, respectively.

  qRangeSlider inherits a comprehensive set of signals:
  \table
  \header \o Signal \o Description
  \row \o \l valueChanged()
  \o Emitted when the slider's value has changed. The tracking()
  determines whether this signal is emitted during user
  interaction.
  \row \o \l sliderPressed()
  \o Emitted when the user starts to drag the slider.
  \row \o \l sliderMoved()
  \o Emitted when the user drags the slider.
  \row \o \l sliderReleased()
  \o Emitted when the user releases the slider.
  \endtable

  qRangeSlider only provides integer ranges. Note that although
  qRangeSlider handles very large numbers, it becomes difficult for users
  to use a slider accurately for very large ranges.

  A slider accepts focus on Tab and provides both a mouse wheel and a
  keyboard interface. The keyboard interface is the following:

  \list
  \o Left/Right move a horizontal slider by one single step.
  \o Up/Down move a vertical slider by one single step.
  \o PageUp moves up one page.
  \o PageDown moves down one page.
  \o Home moves to the start (mininum).
  \o End moves to the end (maximum).
  \endlist

  \table 100%
  \row \o \inlineimage macintosh-slider.png Screenshot of a Macintosh slider
  \o A slider shown in the \l{Macintosh Style Widget Gallery}{Macintosh widget style}.
  \row \o \inlineimage windows-slider.png Screenshot of a Windows XP slider
  \o A slider shown in the \l{Windows XP Style Widget Gallery}{Windows XP widget style}.
  \row \o \inlineimage plastique-slider.png Screenshot of a Plastique slider
  \o A slider shown in the \l{Plastique Style Widget Gallery}{Plastique widget style}.
  \endtable

  \sa QScrollBar, QSpinBox, QDial, {fowler}{GUI Design Handbook: Slider}, {Sliders Example}
*/


/*!
  \enum qRangeSlider::TickPosition

  This enum specifies where the tick marks are to be drawn relative
  to the slider's groove and the handle the user moves.

  \value NoTicks Do not draw any tick marks.
  \value TicksBothSides Draw tick marks on both sides of the groove.
  \value TicksAbove Draw tick marks above the (horizontal) slider
  \value TicksBelow Draw tick marks below the (horizontal) slider
  \value TicksLeft Draw tick marks to the left of the (vertical) slider
  \value TicksRight Draw tick marks to the right of the (vertical) slider

  \omitvalue NoMarks
  \omitvalue Above
  \omitvalue Left
  \omitvalue Below
  \omitvalue Right
  \omitvalue Both
*/


/*!
  Constructs a vertical slider with the given \a parent.
*/
qRangeSlider::qRangeSlider(QWidget* parent)
  : QWidget(parent)
{
  init(Qt::Vertical);
}

/*!
  Constructs a slider with the given \a parent. The \a orientation
  parameter determines whether the slider is horizontal or vertical;
  the valid values are Qt::Vertical and Qt::Horizontal.
*/

qRangeSlider::qRangeSlider(Qt::Orientation orientation, QWidget* parent)
  : QWidget(parent)
{
  init(orientation);
}

void qRangeSlider::init(Qt::Orientation orientation)
{
  unitConverter_ = 0;
  styleOptionRangeSlider_.setOrientation(orientation);
  styleOptionRangeSlider_.setTickInterval(1);
  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  styleOptionRangeSlider_.setCutoffRange(range_t(0, 100));
  styleOptionRangeSlider_.setRange(range_t(40, 60));
  tracking = QStyle::SC_None;
}

/*!
  Destroys this slider.
*/
qRangeSlider::~qRangeSlider()
{
}

QPolygon QStyleRangeSlider::getMarkerArea(const QRect& bbox,
                                          const QPair<int, int>& range,
                                          const QPair<int, int>& cutoffRange,
                                          QStyleRangeSlider::WHICH which) const
{
  QPolygon paintArea;
  int width = markerWidth;

    int pos;
    if (which & FIRST) {
      pos = getPosMin(bbox, range.first, cutoffRange);
      width = -width;
    }
    else {
      assert(which & SECOND);
      pos = getPosMin(bbox, range.second, cutoffRange);
    }

    int y = getGrooveY(bbox);
    int base = y + getGrooveHeight(bbox);
    y -= tipOffset/2;
    int tip = base + tipOffset;

    paintArea.push_back(QPoint(pos, y));
    paintArea.push_back(QPoint(pos, tip));
    paintArea.push_back(QPoint(pos + width, base));
    paintArea.push_back(QPoint(pos + width, y));
    paintArea.push_back(paintArea[0]);

    return paintArea;
  }

void cubic_average(QPolygonF& p, size_t j) {
  p[j] = (p[j-1] + 4 * p[j] + p[j+1]) * (1.0 / 6.0);
}

void point_insert(QPolygonF& p, size_t j, float l = 0.5) {
  size_t prev = j - 1;
  if(j == 0) {
    assert(p[p.size() - 1] == p[0]);
    prev = p.size() - 2;
  }

  QPointF newPoint = (1 - l) * p[prev] + l * p[j];
  p.insert(j, 1, newPoint);

  if(j == 0) {
    p[p.size() - 1] = p[0];
  }
}

void cubic_subdivide(QPolygonF& p, size_t j, int recurse = 1)
{
  assert(j > 0);
  assert(j < uint(p.size()) - 1);


  cubic_average(p, j);

  if(--recurse >= 0) {
    point_insert(p, j + 1);
    point_insert(p, j);
    cubic_subdivide(p, j + 1, recurse);
    cubic_subdivide(p, j, recurse);
  }

}

void QStyleRangeSlider::paintGroove(QPainter& p, const QRect& bbox) const
{
  QRect paintBox;

  paintBox.setX(getGrooveX(bbox));
  paintBox.setY(getGrooveY(bbox));
  paintBox.setWidth(getGrooveWidth(bbox));
  paintBox.setHeight(getGrooveHeight(bbox));


  QPolygonF paintArea = QPolygon(paintBox, true);

  float aspect =
    getGrooveWidth(bbox) * 1.0 /
    getGrooveHeight(bbox);

  float offset = 0.2;
  float offset2 = offset / aspect;

  for (int i = 4; i > 0; --i) {
    if(i % 2 == 0) {
      point_insert(paintArea, i, offset);
      point_insert(paintArea, i-1, 1 - offset2);
    }
    else {
      point_insert(paintArea, i, offset2);
      point_insert(paintArea, i - 1, 1 - offset);
    }
    cubic_subdivide(paintArea, i, 7);
  }

  QRectF pbbox = paintArea.boundingRect();

  QPointF p1 = pbbox.topLeft();
  QPointF p2 = pbbox.bottomRight();
  p2.setX(p1.x());

  QLinearGradient grad(p1, p2);
  grad.setColorAt(0, Qt::black);
  grad.setColorAt(0.15, Qt::gray);
  grad.setColorAt(0.85, Qt::gray);
  grad.setColorAt(1.0, Qt::white);

  //p.fillRect(bbox, Qt::blue);
  p.save();
  //p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::HighQualityAntialiasing, true);
  p.setPen(Qt::NoPen);
  //p.drawPoints(paintArea);
  p.setBrush(QBrush(grad));
  p.drawPolygon(paintArea);
  p.restore();
}

void
QStyleRangeSlider::paintFilling(QPainter& p, const QRect& bbox,
                                const QPair<int, int>& range,
                                const QPair<int, int>& cutoffRange) const
{

  QRect paintBox;

  int x = getPosMin(bbox, range.first, cutoffRange);
  int width = getPosMax(bbox, range.second, cutoffRange) - x;

  paintBox.setX(x);
  paintBox.setY(getGrooveY(bbox));
  paintBox.setWidth(width);
  paintBox.setHeight(getGrooveHeight(bbox));

  QPolygon paintArea = QPolygon(paintBox, true);

  QRectF pbbox = paintArea.boundingRect();

  QPointF p1 = pbbox.topLeft();
  QPointF p2 = pbbox.bottomRight();
  p2.setX(p1.x());

  QLinearGradient grad(p1, p2);
  grad.setColorAt(0, Qt::black);
  grad.setColorAt(0.15, Qt::red);
  grad.setColorAt(0.85, Qt::red);
  grad.setColorAt(1.0, Qt::white);

  p.save();
  //p.setRenderHint(QPainter::Antialiasing, true);
  p.setRenderHint(QPainter::HighQualityAntialiasing, true);
  p.setPen(Qt::NoPen);
  //p.drawPoints(paintArea);
  p.setBrush(QBrush(grad));
  p.drawPolygon(paintArea);
  p.restore();

}

void QStyleRangeSlider::paintMarker(QPainter& p,
                                    const QRect& bbox,
                                    const QPair<int, int>& range,
                                    const QPair<int, int>& cutoffRange,
                                    QStyleRangeSlider::WHICH which) const
{

  QRect paintBox;

  p.setBrush(Qt::lightGray);

  if (which & FIRST) {
    QPolygon poly =
      getMarkerArea(bbox, range, cutoffRange, which);
    p.drawPolygon(poly);
  }
  if (which & SECOND) {
    QPolygon poly =
      getMarkerArea(bbox, range, cutoffRange, which);
    p.drawPolygon(poly);
  }
}

void QStyleRangeSlider::paintTicks(QPainter& p, const QRect& bbox,
                                   const QPair<int, int>& cutoffRange,
                                   float tickInterval /*, bool isLogarithmic*/ ) const
{
  int top = getGrooveY(bbox) + getGrooveHeight(bbox);
  int bottom = bbox.height();

//  top = 0;

  int baseX = getGrooveX(bbox);
  int width = getGrooveWidth(bbox);

  p.setPen(Qt::black);
  double min = cutoffRange.first;
  double max = cutoffRange.second;

  float div = max - min;
  float interval = tickInterval;

  if (tickInterval == 0)
    return;

  if (tickInterval < double(div) / width)
    tickInterval = double(div) / width;

  p.drawLine(QPoint(baseX, top), QPoint(baseX, bottom));


  for (float i = interval; i <= div; i += interval) {
    int x = int(baseX + i * width / div);
    p.drawLine(QPoint(x, top), QPoint(x, bottom));
  }
}


/*!
  \reimp
*/
void qRangeSlider::paintEvent(QPaintEvent*)
{
  QPainter p(this);
//  QRect bbox = getBBox();

  styleRangeSlider()->drawComplexControl(QStyle::CC_CustomBase,
                                         &styleOptionRangeSlider_,
                                         &p,
                                         this);



}

/*!
  \reimp
*/

bool qRangeSlider::event(QEvent* event)
{
  switch(event->type()) {
  case QEvent::HoverEnter:
  case QEvent::HoverLeave:
  case QEvent::HoverMove:
    /*
      if (const QHoverEvent *he = static_cast<const QHoverEvent *>(event))
      d->updateHoverControl(he->pos());
    */
    break;
  case QEvent::StyleChange:
  case QEvent::MacSizeChange:
    //d->resetLayoutItemMargins();
    break;
  default:
    break;
  }
  return QWidget::event(event);
}

QRect qRangeSlider::getBBox() const
{
  QRect bbox = geometry();
  bbox.moveTopLeft(QPoint(0, 0));
  return bbox;
}

/*!
  \reimp
*/
void qRangeSlider::mousePressEvent(QMouseEvent* ev)
{
  QStyle::SubControl elem =
    styleRangeSlider()->hitTestComplexControl(QStyle::CC_CustomBase,
                                              &styleOptionRangeSlider_,
                                              ev->pos(), this);

  if (elem == QStyle::SC_None)
    return;

  ev->accept();
  tracking = elem;

//   qDebug() << "Tracking ";
  switch(tracking) {
  case QStyle::SC_None:
//     qDebug() << "NONE";
    break;
  case QStyle::SC_SliderHandle:
//     qDebug() << "FIRST";
    break;
  case QStyleRangeSlider::SC_SliderHandle2:
//     qDebug() << "SECOND";
    break;
  default:
    abort();
  }

  showValueTooltip();
}

/*!
  \reimp
*/
void qRangeSlider::mouseMoveEvent(QMouseEvent* ev)
{
  if (tracking != QStyle::SC_None) {
    ev->accept();
    if (cutoffRange().second == cutoffRange().first)
      return;

    const QStyleRangeSlider* style = styleRangeSlider();
    QRect bbox = getBBox();
    int x = ev->pos().x();
    int min = style->getGrooveX(bbox);
    int max = min + style->getGrooveWidth(bbox);
    if (x > max)
      x = max;
    int val =
      style->sliderPositionFromValue(
        min, max, x, cutoffRange().second-cutoffRange().first);

    if (val < cutoffRange().first)
      val = cutoffRange().first;
    if (val > cutoffRange().second)
      val = cutoffRange().second;

    range_t newRange = range();

    if (tracking == QStyle::SC_SliderHandle) {
      newRange.first = val;
      if (val > newRange.second)
        newRange.second = val;
    }
    else if(tracking == QStyleRangeSlider::SC_SliderHandle2) {
      newRange.second = val;
      if (val < newRange.first)
        newRange.first = val;
    }
    setRange(newRange);
    showValueTooltip();
  }
}

void qRangeSlider::showValueTooltip()
{
  QPoint pos;
  range_t myRange = range();
  const QStyleRangeSlider* style = styleRangeSlider();
  QRect bbox = getBBox();
  pos.setX(style->sliderValueFromPosition(
                         style->getGrooveX(bbox),
                         style->getGrooveX(bbox) + style->getGrooveWidth(bbox),
                         int(0.5*(myRange.first + myRange.second)),
                         cutoffRange().second-cutoffRange().first));

  QVariant first, second;
  if (unitConverter_) {
    first = unitConverter_->convertFromBase(myRange.first);
    second = unitConverter_->convertFromBase(myRange.second);
  }
  else {
    first = myRange.first;
    second = myRange.second;
  }
  QString text =
    QString("(%1, %2)").arg(first.toString()).arg(second.toString());

  QToolTip::showText(mapToGlobal(pos), text, this, QRect());
}

void qRangeSlider::clamp(int& val, const range_t& clampTo)
{
  if (val < clampTo.first)
    val = clampTo.first;
  if (val > clampTo.second)
    val = clampTo.second;
}

void qRangeSlider::clamp(range_t& value, const range_t& clampTo)
{
  clamp(value.first, clampTo);
  clamp(value.second, clampTo);
}

void qRangeSlider::setRange(const QPair<int,int>& range_in) {
  range_t range = range_in;
  clamp(range, cutoffRange());
  if(this->range() != range) {
    styleOptionRangeSlider_.setRange(range);
    update();
    emit rangeChanged(range);
  }
}

void qRangeSlider::setCutoffRange(const QPair<int, int>& cutoffRange) {
  if(this->cutoffRange() != cutoffRange) {
    styleOptionRangeSlider_.setCutoffRange(cutoffRange);
    setRange(this->range());
    update();
    emit cutoffRangeChanged(cutoffRange);
  }
}

/*!
  \reimp
*/
void qRangeSlider::mouseReleaseEvent(QMouseEvent*)
{
  tracking = QStyle::SC_None;
}

/*!
  \reimp
*/
#ifndef Q_OS_MAC
QSize qRangeSlider::sizeHint() const
{
  //Q_D(const qRangeSlider);
  ensurePolished();
  const int SliderLength = 84;
  int thick = styleRangeSlider()->pixelMetric(QStyle::PM_SliderThickness,
                                   &styleOptionRangeSlider_, this);
  int w = thick, h = SliderLength;
  if (orientation() == Qt::Horizontal) {
    w = SliderLength;
    h = thick;
  }
  return styleRangeSlider()->sizeFromContents(QStyle::CT_Slider,
                                   &styleOptionRangeSlider_,
                                   QSize(w, h),
                                   this).expandedTo(
                                                 QApplication::globalStrut());
}
#endif
/*!
  \reimp
*/
QSize qRangeSlider::minimumSizeHint() const
{
  QSize s = sizeHint();
  int length = 25;
  if (orientation() == Qt::Horizontal)
    s.setWidth(length);
  else
    s.setHeight(length);
  return s;
}

/*!
  \property qRangeSlider::tickPosition
  \brief the tickmark position for this slider

  The valid values are described by the qRangeSlider::TickPosition enum.

  The default value is \l qRangeSlider::NoTicks.

  \sa tickInterval
*/

void qRangeSlider::setTickPosition(QSlider::TickPosition position)
{
  assert(position == QSlider::TicksBelow &&
         "Only QSlider::TicksBelow is implemented");
  styleOptionRangeSlider_.setTickPosition(position);
  update();
  updateGeometry();
}

QSlider::TickPosition qRangeSlider::tickPosition() const
{
  return styleOptionRangeSlider_.tickPosition();
}

/*!
  \fn TickPosition qRangeSlider::tickmarks() const
  \compat

  Use tickPosition() instead.
*/

/*!
  \fn qRangeSlider::setTickmarks(TickPosition position)
  \compat

  Use setTickPosition() instead.
*/

/*!
  \property qRangeSlider::tickInterval
  \brief the interval between tickmarks

  This is a value interval, not a pixel interval. If it is 0, the
  slider will choose between lineStep() and pageStep().

  The default value is 0.

  \sa tickPosition, lineStep(), pageStep()
*/

void qRangeSlider::setTickInterval(float ts)
{
  styleOptionRangeSlider_.setTickInterval(ts);
  update();
}

float qRangeSlider::tickInterval() const
{
  return styleOptionRangeSlider_.tickInterval();
}

/*!
  \fn void qRangeSlider::addStep()

  Use setValue() instead.
*/

/*!
  \fn void qRangeSlider::subtractStep()

  Use setValue() instead.
*/


const QStyleRangeSlider* qRangeSlider::styleRangeSlider() const
{
  if (!styleRangeSlider_)
    styleRangeSlider_ = new QStyleRangeSlider();

  styleRangeSlider_->setRealStyle(qApp->style());
  return styleRangeSlider_;
}


void
qRangeSlider::setUnitConverter(const RangeSliderUnitConverter* unitConverter)
{
  unitConverter_ = unitConverter;
}

void qRangeSlider::setLogarithmic(bool logarithmic)
{
  styleOptionRangeSlider_.setLogarithmic(logarithmic);
}

bool qRangeSlider::isLogarithmic() const
{
  return styleOptionRangeSlider_.isLogarithmic();
}

#endif
