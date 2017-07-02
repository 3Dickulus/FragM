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

#ifndef QRANGESLIDER_H
#define QRANGESLIDER_H

#include <QStyle>
#include <QSlider>
#include <QStyleOptionComplex>
#include <QWidget>

/**
 * Used for converting native units, to some logical unit
 */
class RangeSliderUnitConverter {
public:
  /**
   * Destructs the RangeSliderUnitConverter
   */
  virtual ~RangeSliderUnitConverter() {}

  /**
   * Convert from the base unit to logical unit
   *
   * @param baseunit The unit of the base
   */
  virtual QVariant convertFromBase(int baseunit) const = 0;

  /**
   * Converts from a logical unit to int
   *
   * @param logical Some representation that can be converted to an int
   */
  virtual int convertToBase(QVariant logical) const = 0;
};

class QStyleOptionRangeSlider : public QStyleOptionComplex {
  public:
  typedef QPair<int,int> range_t;
  void setTickPosition(QSlider::TickPosition position) {
    tickPosition_ = position;
  }

  QSlider::TickPosition tickPosition() const
  {
    return tickPosition_;
  }

  const range_t& range() const
  {
    return range_;
  }

  const range_t& cutoffRange() const
  {
    return cutoffRange_;
  }

  void setOrientation(Qt::Orientation orientation)
  {
    orientation_ = orientation;
  }

  void setCutoffRange(const range_t& cutoffRange)
  {
    cutoffRange_ = cutoffRange;
  }

  void setRange(const range_t& range)
  {
    range_ = range;
  }

  Qt::Orientation orientation() const
  {
    return orientation_;
  }

  void setTickInterval(float tickInterval)
  {
    tickInterval_ = tickInterval;
  }

  float tickInterval() const
  {
    return tickInterval_;
  }

  void setLogarithmic(bool logarithmic)
  {
    logarithmic_ = logarithmic;
  }

  bool isLogarithmic() const
  {
    return logarithmic_;
  }

private:
  range_t range_;
  range_t cutoffRange_;

  float tickInterval_;
  bool logarithmic_;

  QSlider::TickPosition tickPosition_;
  Qt::Orientation orientation_;
};

class QStyleRangeSlider;

class qRangeSlider : public QWidget
{
  Q_OBJECT

  typedef QPair<int,int> range_t;

  //    Q_ENUMS(TickPosition)
  Q_PROPERTY(range_t range READ range WRITE setRange )
  Q_PROPERTY(range_t cutoffRange READ cutoffRange WRITE setCutoffRange )
  Q_PROPERTY(QSlider::TickPosition tickPosition
  READ tickPosition
  WRITE setTickPosition)
  Q_PROPERTY(float tickInterval READ tickInterval WRITE setTickInterval)

public:
//  qRangeSlider(QWidget *parent = 0);
  enum ELEMENT { NONE = 0x0, FIRST = 0x1, SECOND = 0x2};

  enum TickPosition {
    NoTicks = 0,
    TicksAbove = 1,
    TicksLeft = TicksAbove,
    TicksBelow = 2,
    TicksRight = TicksBelow,
    TicksBothSides = 3

  };

  explicit qRangeSlider(QWidget* parent = 0);
  explicit qRangeSlider(Qt::Orientation orientation, QWidget* parent = 0);

  ~qRangeSlider();

#ifndef __APPLE__
  QSize sizeHint() const;
#endif
  QSize minimumSizeHint() const;

  void setTickPosition(QSlider::TickPosition position);
  QSlider::TickPosition tickPosition() const;

  void setTickInterval(float ti);
  float tickInterval() const;

  bool event(QEvent* event);

  void setUnitConverter(const RangeSliderUnitConverter* = 0);

  const range_t& range() const {
    return styleOptionRangeSlider_.range();
  }

  const range_t& cutoffRange() const {
    return styleOptionRangeSlider_.cutoffRange();
  }

  void setLogarithmic(bool logarithmic);
  bool isLogarithmic() const;

  QRect getBBox() const;
signals:
  void rangeChanged(QPair<int, int> range);
  void cutoffRangeChanged(QPair<int, int> range);

public slots:
  void setRange(const QPair<int, int>& value);
  void setCutoffRange(const QPair<int, int>& value);

protected:
  void init(Qt::Orientation orientation);

  void paintEvent(QPaintEvent* ev);
  void mousePressEvent(QMouseEvent* ev);
  void mouseReleaseEvent(QMouseEvent*);
  void mouseMoveEvent(QMouseEvent* ev);
  void initStyleOption(QStyleOptionRangeSlider* option);
  const QStyleRangeSlider* styleRangeSlider() const;
  Qt::Orientation orientation() const
  {
    return styleOptionRangeSlider_.orientation();
  }

  void showValueTooltip();


private:
  Q_DISABLE_COPY(qRangeSlider)

  QStyle::SubControl tracking;


  QStyleOptionRangeSlider styleOptionRangeSlider_;

  static void clamp(int& value, const range_t& clamp_to);
  static void clamp(range_t& value, const range_t& clamp_to);

private:
  static QStyleRangeSlider* styleRangeSlider_;
  const RangeSliderUnitConverter* unitConverter_;
};

#endif
