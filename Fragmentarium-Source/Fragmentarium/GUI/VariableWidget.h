#pragma once

#include <QDebug>
#include <QString>
#include <QVector>
#include <QWidget>
#include <QComboBox>
#include <QMap>
#include <QSlider>
#include <QTabWidget>
#include <QFrame>
#include <QCheckBox>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>

#include <QOpenGLShaderProgram>
#include "../Parser/Preprocessor.h"
#include "SyntopiaCore/Logging/Logging.h"
#include "DisplayWidget.h"

/// Classes for the GUI Editor for the preprocessor constant variables.
/// E.g. the line: uniform float Angle; slider[45.00,0,360]
///	will make a simple editor widget appear.
namespace Fragmentarium {
    namespace GUI {

// decimal places for float and double
#define FDEC 9
#define DDEC 18
        
        using namespace SyntopiaCore::Logging;
        using namespace Fragmentarium::Parser;

        // A helper class (combined float slider+spinner)
        class ComboSlider : public QWidget {
            Q_OBJECT
            Q_PROPERTY(double value READ getValue WRITE setValue)
        public:
            ComboSlider(QWidget* parent, QWidget* variableEditor, double defaultValue, double minimum, double maximum)
                : QWidget(parent), variableEditor(variableEditor), defaultValue(defaultValue), minimum(minimum), maximum(maximum){
    
                setMinimumSize(160,20);
                setMaximumSize(1024,20);
                Logarithmic = false;
                
                QHBoxLayout* l = new QHBoxLayout(this);
                l->setSpacing(2);
                l->setContentsMargins(0,0,0,0);

                // 4294967295
                scale = (1.0/(maximum-minimum))*(int(__INT32_MAX__*0.5)+1); 

                slider = new QSlider(Qt::Horizontal,this);
                slider->setRange(minimum*scale,maximum*scale);
                slider->setValue(defaultValue*scale);
                slider->setSingleStep(scale/1000);
                slider->setPageStep(scale/100);
                slider->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum));
                l->addWidget(slider);

                spinner = new QDoubleSpinBox(this);
                spinner->setDecimals(FDEC);
                spinner->setMaximum(maximum);
                spinner->setMinimum(minimum);
                spinner->setValue(defaultValue);
                spinner->setKeyboardTracking(false);
                l->addWidget(spinner);
                 
                setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum));

                connect(spinner, SIGNAL(valueChanged(double)), this, SLOT(spinnerChanged(double)));
                connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));

                m_anim = new QPropertyAnimation(this,"value");
                m_loops = 1;
                m_pong = 0;
                m_framestart = 0;
                m_framefin = 0;
            }
            void setDecimals(int d) {spinner->setDecimals(d);};
            void contextMenuEvent(QContextMenuEvent* ) {
             
              bool ok;
              int i = QInputDialog::getInt(this, objectName(),
                                           tr("Slider Step Multiplier"), slider->singleStep(), 1, 100000000, 1, &ok);
              if (ok){
                slider->setSingleStep(i);
                slider->setPageStep(i*10);
              }
            }
            
            QPropertyAnimation *m_anim;
            int m_framestart, m_framefin; // firstframe lastframe
            int m_loops;
            int m_pong;
            
            double getMin() { return minimum; }
            double getMax() { return maximum; }
            double getSpan() { return maximum-minimum; }
            double getValue() { return spinner->value(); }

            int getLoops() { return m_loops; }
            int getPong() { return m_pong; }

        public slots:
            void setLogarithmic( bool l) {
                Logarithmic=l;
                if(!l) return;
                scale = ((maximum-minimum) / __INT32_MAX__);
                slider->setRange(minimum*scale,maximum*scale);
                slider->setSingleStep(scale*1000);
                slider->setPageStep(scale*10000);
            }

            void setValue(double d) {
                spinner->setValue(d);
                bool clamping = (d < minimum || d > maximum);
                if(clamping) {
                    QString msg = QString( tr("Clamping") + " " + objectName() + " " + tr("to min/max range!") );
                    WARNING( msg );
                }
            }

        signals:
            void changed();

        protected slots:
            void spinnerChanged(double d) {
                // our value comes from the spin box
                // the slider must be kept quiet while adjusting from spinner
                slider->blockSignals(true);
                slider->setValue( Logarithmic ? slider->minimum() + (log(d) / scale) : d*scale);
                slider->blockSignals(false);
                // let the main gui thread know something happened
                 emit changed();
              
            }
            void sliderChanged(int i) {
                // our value is calculated from the slider position
                // the spinner must be kept quiet while adjusting from slider
                spinner->blockSignals(true);
                spinner->setValue(Logarithmic ? minimum + exp(i*scale) : i/scale);
                spinner->blockSignals(false);
                // let the main gui thread know something happened
                emit changed();
            }

        private:
            QSlider* slider;
            QDoubleSpinBox* spinner;
            QWidget* variableEditor;
            double defaultValue;
            double minimum;
            double maximum;
            double scale;
            bool Logarithmic;
        };

        class ColorChooser : public QFrame {
            Q_OBJECT
        public:
            ColorChooser(QWidget* parent, QVector3D defaultValue)
                : QFrame(parent), defaultValue(defaultValue), value(defaultValue) {
                setLayout(new QHBoxLayout(this));
                setColor(defaultValue);
                setFrameStyle(QFrame::Panel | QFrame::Plain);
                setLineWidth(1);
            }

        public slots:
            void setColor(QVector3D v) {
                QPalette p = palette();
                QColor c;
                c.setRgbF(v.x(),v.y(),v.z());
                p.setColor(backgroundRole(), c);
                setAutoFillBackground( true );
                setPalette(p);
                value = v;
            }
            
            void setColor( QColor c ) {
                if (c.isValid()) {
                    value = QVector3D(c.redF(), c.greenF(), c.blueF());
                    setColor(value);
                    emit changed();
                }
            }

            QVector3D getValue() { return value; }

        private slots:
            void mouseReleaseEvent(QMouseEvent*) {
                QColor initial;
                initial.setRgbF( value.x(), value.y(), value.z() );

                qcd = new QColorDialog(initial);
                connect(qcd,SIGNAL(currentColorChanged(QColor)),this,SLOT(setColor(QColor)));
                if(!qcd->exec()) setColor(initial);
            }

        signals:
            void changed();

        private:
            QColorDialog *qcd;
            QVector3D defaultValue;
            QVector3D value;
        };


        // A helper class (combined int slider+spinner)
        class IntComboSlider : public QWidget {
            Q_OBJECT
        public:
            IntComboSlider(QWidget* parent, int defaultValue, int minimum, int maximum)
                : QWidget(parent), defaultValue(defaultValue), min(minimum), max(maximum){
                  
                QHBoxLayout* l = new QHBoxLayout(this);
                l->setSpacing(2);
                l->setContentsMargins(0,0,0,0);

                slider = new QSlider(Qt::Horizontal,this);
                slider->setRange(minimum,maximum);
                slider->setValue(defaultValue);
                l->addWidget(slider);

                spinner = new QSpinBox(this);
                spinner->setMaximum(maximum);
                spinner->setMinimum(minimum);
                spinner->setValue(defaultValue);
                spinner->setKeyboardTracking(false);
                l->addWidget(spinner);

                setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum));

                connect(spinner, SIGNAL(valueChanged(int)), this, SLOT(spinnerChanged(int)));
                connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
            }

            int getValue() { return spinner->value(); }
        public slots:
            void setValue(int i) { spinner->setValue(i<min?min:i>max?max:i); }

	signals:
            void changed();

        protected slots:
            void spinnerChanged(int val) {
                slider->blockSignals(true);
                slider->setValue(val);
                slider->blockSignals(false);
                emit changed();
            }

            void sliderChanged(int val) {
                spinner->blockSignals(true);
                spinner->setValue(val);
                spinner->blockSignals(false);
                emit changed();
            }

        private:

            QSlider* slider;
            QSpinBox* spinner;
            int defaultValue;
            int min;
            int max;
        };


        class CameraControl;

        /// Widget editor base class.
        class VariableWidget : public QWidget {

            Q_OBJECT
        public:
            VariableWidget(QWidget* parent, QWidget* variableEditor, QString name);
            virtual void updateTextures(FragmentSource* /*fs*/,  FileManager* /*fileManager*/) {}
            virtual QString getValueAsText() { return ""; }
            QString getName() const { return name; }
            virtual void reset() = 0;
            void setGroup(QString group) { this->group = group; }
            QString getGroup() { return group; }
            bool isUpdated() const { return updated; }
            void setUpdated(bool value) { updated = value; }
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram) = 0;
            QString toSettingsString();
            bool fromSettingsString(QString string);
            virtual QString toString() = 0;
            virtual void fromString(QString string) = 0;
            virtual QString getUniqueName() = 0;
            void setSystemVariable(bool v) { systemVariable = v; }
            bool isSystemVariable() { return systemVariable; }
            virtual void setIsDouble(bool v) = 0;
            bool isDouble() { return wantDouble; }
            int uniformLocation(QOpenGLShaderProgram* shaderProgram);
            bool isLocked();
            LockType getDefaultLockType() { return defaultLockType; }
            void setDefaultLockType(LockType lt);
            LockType getLockType() { return lockType; }
            virtual void setLockType(LockType lt) ;
            virtual QString getLockedSubstitution() = 0;
            virtual QString getLockedSubstitution2() = 0;

        public slots:
            void locked(bool l);
            void valueChanged();

        signals:
            void changed(bool lockedChanged);

        protected:
            QString toGLSL(double d) {
                QString n = QString::number(d,'g',isDouble()?DDEC:FDEC);
                // GLSL requires a dot in floats.
                if (n.contains(".") || n.contains("e"))
                    return n;
                return n+".0";
            }

            LockType defaultLockType;
            LockType lockType;
            QPushButton* lockButton;
            QString name;
            QString group;
            bool updated;
            bool systemVariable;
            bool wantDouble=false;
            QWidget* widget;
            QWidget* variableEditor;
        };

        class SamplerWidget : public VariableWidget {
            Q_OBJECT
        public:
            SamplerWidget(FileManager* fileManager, QWidget* parent, QWidget* variableEditor, QString name, QString defaultValue);
            virtual QString toString();
            virtual void fromString(QString string);
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
            virtual void updateTextures(FragmentSource* fs, FileManager* fileManager);
            virtual void setLockType(LockType /*lt*/) { lockType = AlwaysLocked; } // cannot change this
            QString getValue() ;
            virtual QString getUniqueName() { return QString("%1:%2:%3:%4").arg(group).arg(getName()); }
            void reset() { comboBox->setEditText(defaultValue); }
            QString getLockedSubstitution() { return QString(); }
            QString getLockedSubstitution2() { return QString(); }
            void setIsDouble( bool wd = false) {
                wantDouble = wd;
            };
            int texID;

        signals:
            void changed();

        protected slots:

            void textChanged(const QString& text);

            void buttonClicked();

        private:

            QComboBox* comboBox;
            QPushButton* pushButton;
            FileManager* fileManager;
            QString defaultValue;
        };


        /// A widget editor for a float variable.
        class FloatWidget : public VariableWidget {
        public:
            /// FloatVariable constructor.
            FloatWidget(QWidget* parent, QWidget* variableEditor, QString name, double defaultValue, double min, double max);
            virtual QString getUniqueName() { return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(min).arg(max); }
            virtual QString getValueAsText() {
                int p = FDEC;
                if(isDouble()) p = DDEC;
                return QString::number(comboSlider1->getValue(),'g',p);
            }
            virtual QString toString();
            virtual void fromString(QString string);
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
            double getValue() { return comboSlider1->getValue(); }
            void setValue(double f);
            void reset() { setValue(defaultValue); }
            QString getLockedSubstitution() {
                QString type = isDouble() ? "double " : "float ";
                return "const " + type + name + " = " + toGLSL(getValue()) +";";
            }
            QString getLockedSubstitution2() { return "#define " + name + " " + toGLSL(getValue()); }
            void setIsDouble( bool wd = false) {
                wantDouble = wd;
                comboSlider1->setDecimals(wd ? DDEC : FDEC);
            };
        private:
            ComboSlider* comboSlider1;
            double defaultValue;
            double min;
            double max;
        };

        /// A widget editor for a float2 variable.
        class Float2Widget : public VariableWidget {
        public:
            Float2Widget(QWidget* parent, QWidget* variableEditor, QString name, QVector2D defaultValue, QVector2D min, QVector2D max);
 
            virtual QString getUniqueName() {
                QString f = QString("[%1 %2]").arg(min.x()).arg(min.y());
                QString t = QString("[%1 %2]").arg(max.x()).arg(max.y());
                return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(f).arg(t);
            }

            virtual QString getValueAsText() {
                int p = FDEC;
                if(isDouble()) p = DDEC;
                return QString::number(comboSlider1->getValue(),'g',p) + "," + QString::number(comboSlider2->getValue(),'g',p);
            }
            virtual QString toString();
            virtual void fromString(QString string);
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);

            // Third component unused for these
            QVector2D getValue() { return QVector2D(comboSlider1->getValue(),comboSlider2->getValue()); }
            void setValue(QVector3D v);
            void reset() { setValue(defaultValue); }
            QString getLockedSubstitution() {
                QString type = isDouble() ? "dvec2" : "vec2";
                return "const " + type + " " + name + " = " + type + "(" + toGLSL(getValue().x()) + "," + toGLSL(getValue().y()) +");";
            }
            QString getLockedSubstitution2() {
                QString type = isDouble() ? " dvec2(" : " vec2(";
                return "#define " + name + type + toGLSL(getValue().x()) + "," + toGLSL(getValue().y()) +")";
            }
            void setIsDouble( bool wd = false) {
                wantDouble = wd;
                comboSlider1->setDecimals(wd ? DDEC : FDEC);
                comboSlider2->setDecimals(wd ? DDEC : FDEC);
            };

        private:

            ComboSlider* comboSlider1;
            ComboSlider* comboSlider2;
            QVector2D defaultValue;
            QVector2D min;
            QVector2D max;
        };


        /// A widget editor for a float3 variable.
        class Float3Widget : public VariableWidget {
            Q_OBJECT
        public:
            /// FloatVariable constructor.
            Float3Widget(QWidget* parent, QWidget* variableEditor, QString name, QVector3D defaultValue, QVector3D min, QVector3D max);
            virtual QString getUniqueName();
            virtual QString getValueAsText() {
                int p = FDEC;
                if(isDouble()) p = DDEC;
                return QString::number(comboSlider1->getValue(),'g',p) + "," + QString::number(comboSlider2->getValue(),'g',p) + "," + QString::number(comboSlider3->getValue(),'g',p);
            }
            virtual QString toString();
            virtual void fromString(QString string);
            QVector3D getValue() { return QVector3D(comboSlider1->getValue(),comboSlider2->getValue(),comboSlider3->getValue()); }
            void setValue(QVector3D v);
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
            void reset() { setValue(defaultValue); }
            QString getLockedSubstitution() {
                QString type = isDouble() ? "dvec3" : "vec3";
                return "const " + type + " " + name + " = " + type + "(" + toGLSL(getValue().x()) + "," + toGLSL(getValue().y()) + "," + toGLSL(getValue().z()) +");";
            }
            QString getLockedSubstitution2() {
                QString type = isDouble() ? " dvec3(" : " vec3(";
                return "#define " + name + type + toGLSL(getValue().x()) + "," + toGLSL(getValue().y()) + "," + toGLSL(getValue().z()) +")";
            }
            void setIsDouble( bool wd = false) {
                wantDouble = wd;
                comboSlider1->setDecimals(wd?DDEC:FDEC);
                comboSlider2->setDecimals(wd?DDEC:FDEC);
                comboSlider3->setDecimals(wd?DDEC:FDEC);
            };

        public slots:
            void n1Changed();
            void n2Changed();
            void n3Changed();
        signals:
            void doneChanges();
        private:
            bool normalize;
            ComboSlider* comboSlider1;
            ComboSlider* comboSlider2;
            ComboSlider* comboSlider3;
            QVector3D defaultValue;
            QVector3D min;
            QVector3D max;
        };

        /// A widget editor for a float4 variable.
        class Float4Widget : public VariableWidget {
            Q_OBJECT
        public:
            /// FloatVariable constructor.
            Float4Widget(QWidget* parent, QWidget* variableEditor, QString name, QVector4D defaultValue, QVector4D min, QVector4D max);
            virtual QString getUniqueName();
            virtual QString getValueAsText() {
                int p = FDEC;
                if(isDouble()) p = DDEC;
                return QString::number(comboSlider1->getValue(),'g',p) + "," + QString::number(comboSlider2->getValue(),'g',p) + "," + QString::number(comboSlider3->getValue(),'g',p) + "," + QString::number(comboSlider4->getValue(),'g',p);
            }
            virtual QString toString();
            virtual void fromString(QString string);
            QVector4D getValue() { return QVector4D(comboSlider1->getValue(),comboSlider2->getValue(),comboSlider3->getValue(),comboSlider4->getValue()); }
            void setValue(QVector4D v);
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
            void reset() { setValue(defaultValue); }
            QString getLockedSubstitution() {
                QString type = isDouble() ? "dvec4" : "vec4";
                return "const " + type + " " + name + " = " + type + "(" + toGLSL(getValue().x()) + "," + toGLSL(getValue().y()) + "," + toGLSL(getValue().z())+ "," + toGLSL(getValue().w()) +");";
            }
            QString getLockedSubstitution2() {
                QString type = isDouble() ? " dvec4(" : " vec4(";
                return "#define " + name + type + toGLSL(getValue().x()) + "," + toGLSL(getValue().y()) + "," + toGLSL(getValue().z())+ "," + toGLSL(getValue().w()) +")";
            }
            void setIsDouble( bool wd = false) {
                wantDouble = wd;
                comboSlider1->setDecimals(wd?DDEC:FDEC);
                comboSlider2->setDecimals(wd?DDEC:FDEC);
                comboSlider3->setDecimals(wd?DDEC:FDEC);
                comboSlider4->setDecimals(wd?DDEC:FDEC);
            };

        signals:
            void doneChanges();
        private:
            bool normalize;
            ComboSlider* comboSlider1;
            ComboSlider* comboSlider2;
            ComboSlider* comboSlider3;
            ComboSlider* comboSlider4;
            QVector4D defaultValue;
            QVector4D min;
            QVector4D max;
        };


        class ColorWidget : public VariableWidget {
        public:
            /// FloatVariable constructor.
            ColorWidget(QWidget* parent, QWidget* variableEditor, QString name, QVector3D defaultValue);
            virtual QString getUniqueName() { return QString("%1:%2").arg(group).arg(getName()); }
            virtual QString getValueAsText() {
                QVector3D t = colorChooser->getValue();
                int p = FDEC;
                if(isDouble()) p = DDEC;
                return QString::number(t.x(),'g',p) + "," + QString::number(t.y(),'g',p) + "," + QString::number(t.z(),'g',p);
            }

            void setValue(QVector3D v);
            virtual QString toString();
            virtual void fromString(QString string);
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
            void reset() { colorChooser->setColor(defaultValue); }
            QString getLockedSubstitution() {
                QString type = isDouble() ? "dvec3" : "vec3";
                return "const " + type + " " + name + " = " + type + "(" + toGLSL(colorChooser->getValue().x()) + "," + toGLSL(colorChooser->getValue().y()) + "," + toGLSL(colorChooser->getValue().z()) +");";
            }
            QString getLockedSubstitution2() {
                QString type = isDouble() ? " dvec3(" : " vec3(";
                return "#define " + name + type + toGLSL(colorChooser->getValue().x()) + "," + toGLSL(colorChooser->getValue().y()) + "," + toGLSL(colorChooser->getValue().z()) +")";
            }
            void setIsDouble( bool wd = false) {
                wantDouble = wd;
            };
        private:
            ColorChooser* colorChooser;
            QVector3D defaultValue;
        };

        class FloatColorWidget : public VariableWidget {
        public:
            /// FloatVariable constructor.
            FloatColorWidget(QWidget* parent, QWidget* variableEditor, QString name, double defaultValue, double min, double max, QVector3D defaultColorValue);
            virtual QString getUniqueName() { return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(min).arg(max); }
            virtual QString getValueAsText() {
                QVector3D t = colorChooser->getValue();
                int p = FDEC;
                if(isDouble()) p = DDEC;
                return QString::number(t.x(),'g',p) + "," + QString::number(t.y(),'g',p) + "," + QString::number(t.z(),'g',p) + "," + QString::number(comboSlider->getValue(),'g',p );
            }
            void setValue(QVector4D v);
            virtual QString toString();
            virtual void fromString(QString string);
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
            void reset() { comboSlider->setValue(defaultValue); colorChooser->setColor(defaultColorValue); }
            QString getLockedSubstitution() {
                QString type = isDouble() ? "dvec4" : "vec4";
                return "const " + type + " " + name + " = " + type + "(" + toGLSL(colorChooser->getValue().x()) + "," + toGLSL(colorChooser->getValue().y()) + "," + toGLSL(colorChooser->getValue().z())+ "," + toGLSL(comboSlider->getValue()) +");";
            }
            QString getLockedSubstitution2() {
                QString type = isDouble() ? " dvec4(" : " vec4(";
                return "#define " + name + type + toGLSL(colorChooser->getValue().x()) + "," + toGLSL(colorChooser->getValue().y()) + "," + toGLSL(colorChooser->getValue().z())+ "," + toGLSL(comboSlider->getValue()) +")";
            }
            void setIsDouble( bool wd = false) {
                wantDouble = wd;
                comboSlider->setDecimals(wd ? DDEC : FDEC);
            };
        private:
            ComboSlider* comboSlider;
            ColorChooser* colorChooser;
            double defaultValue;
            double min;
            double max;
            QVector3D defaultColorValue;
        };

        class IntWidget : public VariableWidget {
        public:
            /// IntVariable constructor.
            IntWidget(QWidget* parent, QWidget* variableEditor, QString name, int defaultValue, int min, int max);
            virtual QString getUniqueName() { return QString("%1:%2:%3:%4").arg(group).arg(getName()).arg(min).arg(max); }
            virtual QString getValueAsText() { return QString::number(comboSlider->getValue()); };
            virtual QString toString();
            virtual void fromString(QString string);
            int getValue(){ return comboSlider->getValue(); }
            void setValue( int i){ comboSlider->setValue(i); }
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
            void reset() { comboSlider->setValue(defaultValue); }
            QString getLockedSubstitution() { return "const int " + name + " = " + QString::number(comboSlider->getValue()) + ";"; };
            QString getLockedSubstitution2() { return "#define " + name + " " + QString::number(comboSlider->getValue()) + ""; };
            void setIsDouble( bool wd = false) {
                wantDouble = wd;
            };
        private:
            IntComboSlider* comboSlider;
            int defaultValue;
            int min;
            int max;
        };

        class BoolWidget : public VariableWidget {
        public:
            /// BoolVariable constructor.
            BoolWidget(QWidget* parent, QWidget* variableEditor, QString name, bool defaultValue);
            virtual QString getUniqueName() { return QString("%1:%2").arg(group).arg(getName()); }
            virtual QString getValueAsText() { return (checkBox->isChecked()?"true":"false"); }
            virtual QString toString();
            virtual void fromString(QString string);
            virtual void setUserUniform(QOpenGLShaderProgram* shaderProgram);
            void reset() { checkBox->setChecked(defaultValue); }
            QString getLockedSubstitution() { return "const bool " + name + " = " + (checkBox->isChecked() ? "true" : "false")+ ";"; }
            QString getLockedSubstitution2() { return "#define " + name + " " + (checkBox->isChecked() ? "true" : "false")+ ""; }
            bool isChecked() { return checkBox->isChecked(); }
            void setIsDouble( bool wd = false ) {
                wantDouble = wd;
            };

        private:
            QCheckBox* checkBox;
            bool defaultValue;
        };


    }
}



