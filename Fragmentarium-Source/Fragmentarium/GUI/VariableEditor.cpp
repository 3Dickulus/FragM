#include "VariableEditor.h"

#include <QApplication>
#include <QClipboard>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QSpacerItem>
#include <qglobal.h>

#include "../../SyntopiaCore/Logging/ListWidgetLogger.h"
#include "../../SyntopiaCore/Misc/MiniParser.h"
#include "EasingWindow.h"
#include "MainWindow.h"
#include "VariableWidget.h"


namespace Fragmentarium
{
namespace GUI
{

using namespace SyntopiaCore::Logging;
using namespace SyntopiaCore::Misc;

VariableEditor::VariableEditor(QWidget *parent, MainWindow *mainWin)
    : QWidget(parent), mainWindow(mainWin)
{
    currentComboSlider = nullptr;
    layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins (0,0,0,0);

    QWidget* tw = new QWidget(this);
    auto *topLayout = new QHBoxLayout(tw);
    auto *horizontalSpacer = new QSpacerItem(1024, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    topLayout->addItem(horizontalSpacer);

    QLabel* l = new QLabel(tr("Preset:"), tw);
    topLayout->addWidget(l);
    presetComboBox = new QComboBox(tw);
    connect(presetComboBox, SIGNAL(activated(QString)), this, SLOT(presetSelected(QString)));
    presetComboBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    topLayout->addWidget(presetComboBox);
    QPushButton* pb2 = new QPushButton(tr("Apply"), tw);
    connect(pb2, SIGNAL(clicked()), this, SLOT(applyPreset()));
    topLayout->addWidget(pb2);

    tw->layout()->setContentsMargins(0,0,0,0);
    layout->addWidget(tw);

    tabWidget = new QTabWidget(this);
    layout->addWidget(tabWidget);

    QWidget* w = new QWidget(this);
    new QHBoxLayout(w);
    w->layout()->setContentsMargins(0,0,0,0);
    QPushButton* pb= new QPushButton(tr("Reset All"), this);
    connect(pb, SIGNAL(clicked()), this, SLOT(resetUniforms()));
    w->layout()->addWidget(pb);

    pb= new QPushButton(tr("Copy Settings"), this);
    connect(pb, SIGNAL(clicked()), this, SLOT(copy()));
    w->layout()->addWidget(pb);

    pb= new QPushButton(tr("Paste Settings"), this);
    connect(pb, SIGNAL(clicked()), this, SLOT(paste()));
    w->layout()->addWidget(pb);
    layout->addWidget(w);

    spacer = nullptr;
    currentWidget = nullptr;

    tabWidget->setTabPosition(QTabWidget::East);

    connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)), this, SLOT(focusChanged(QWidget *, QWidget *)));

    easingEnabled=false;
    useDefines=false;
    keyFramesEnabled = false;
    saveEasing = false;
    verbose = false;
}

void VariableEditor::presetSelected(QString presetName)
{
    if(presetName.contains("KeyFrame")) {
        mainWindow->selectPreset();
        mainWindow->getEngine()->update();
    }
}

void VariableEditor::sliderDestroyed(QObject *obj)
{
    if (obj==currentComboSlider) {
        currentComboSlider = nullptr;
    }
}

void VariableEditor::focusChanged(QWidget *oldWidget, QWidget *newWidget)
{

    // Detect if a ComboSlider gets or looses focus
    ComboSlider *oldFloat = nullptr;
    if (oldWidget != nullptr && oldWidget->parent() != nullptr) {
        oldFloat = qobject_cast<ComboSlider *>(oldWidget->parent());
    }
    if (oldFloat == nullptr) {
        oldFloat = qobject_cast<ComboSlider *>(oldWidget);
    }
    ComboSlider *newFloat = nullptr;
    if (newWidget != nullptr && newWidget->parent() != nullptr) {
        newFloat = qobject_cast<ComboSlider *>(newWidget->parent());
    }
    if (newFloat == nullptr) {
        newFloat = qobject_cast<ComboSlider *>(newWidget);
    }

    if (newFloat != nullptr) {

        if (currentComboSlider != nullptr) {
            currentComboSlider->setPalette(QApplication::palette(oldFloat));
            currentComboSlider->setAutoFillBackground(false);
        }

        QPalette pal = newFloat->palette();
        pal.setColor(newFloat->backgroundRole(), Qt::gray);
        newFloat->setPalette(pal);
        newFloat->setAutoFillBackground(true);
        currentComboSlider = newFloat;
        connect(currentComboSlider, SIGNAL(destroyed(QObject *)), this, SLOT(sliderDestroyed(QObject *)));
    }

    QApplication::postEvent(tabWidget, new QEvent(QEvent::LayoutRequest));
}

void VariableEditor::setPresets(QMap<QString, QString> presets)
{
    QString pi = presetComboBox->currentText();
    presetComboBox->clear();
    foreach (QString preset, presets.keys()) {
        presetComboBox->addItem(preset);
    }
    this->presets = presets;

    if (!pi.isEmpty()) {
        presetComboBox->setCurrentIndex(presetComboBox->findText(pi, Qt::MatchContains | Qt::MatchFixedString));
    }

    if (getKeyFrameCount() > 1) {
        mainWindow->initKeyFrameControl();
    }
}

bool VariableEditor::setDefault()
{
    if (!(QSettings().value("autorun", true).toBool())) {
        return false;
    }
    int i = presetComboBox->findText("default", Qt::MatchContains | Qt::MatchFixedString);
    if (i != -1) {
        presetComboBox->setCurrentIndex(i);
        INFO(tr("Found '") + presetComboBox->currentText() + tr("' preset. Executing..."));
        return applyPreset();
    }
    return false;
}

bool VariableEditor::applyPreset()
{
    QString presetName = presetComboBox->currentText();
    QString preset = presets[presetName];
    /// this bit of fudge sets the current time to keyframe time
    QRegExp rx = QRegExp("(KeyFrame\\.\\d\\d\\d)");
    if(rx.indexIn(presetName) != -1)  { /// found a keyframe
        mainWindow->setTimeSliderValue(getCurrentKeyFrame() * ((mainWindow->getTimeMax() * mainWindow->renderFPS) / (getKeyFrameCount() - 1)));
    }
    return setSettings(preset);
}

void VariableEditor::resetUniforms(bool clear)
{
    for (int i = 0; i < variables.count(); i++ ) {
        delete(variables[i]);
    }
    variables.clear();
    mainWindow->resetCamera(true);

    if (clear) {
        mainWindow->initializeFragment();
    }
}

void VariableEditor::copy()
{
    INFO(tr("Copied settings to clipboard"));
    QClipboard *cb = QApplication::clipboard();
    cb->setText( getSettings(),QClipboard::Clipboard );
}

void VariableEditor::paste()
{
    INFO(tr("Pasted settings from clipboard"));
    QClipboard *cb = QApplication::clipboard();
    QString text = cb->text(QClipboard::Clipboard);
    setSettings(text);
}

void VariableEditor::lockGroup()
{
    QWidget* t = tabWidget->widget(tabWidget->currentIndex());

    QMap<QString, QWidget*>::const_iterator it;
    QString g;
    for (it = tabs.constBegin(); it!=tabs.constEnd(); it++ ) {
        if (it.value()->parent() == t) {
            g = it.key();
        }
    }

    foreach (VariableWidget* variable, variables) {
        if (variable->getGroup() == g) {
            variable->locked(true);
        }
    }
}

void VariableEditor::unlockGroup()
{
    QWidget* t = tabWidget->widget(tabWidget->currentIndex());

    QMap<QString, QWidget*>::const_iterator it;
    QString g;
    for (it = tabs.constBegin(); it!=tabs.constEnd(); it++ ) {
        if (it.value()->parent() == t) {
            g = it.key();
        }
    }

    foreach (VariableWidget* variable, variables) {
        if (variable->getGroup() == g) {
            variable->locked(false);
        }
    }
}

void VariableEditor::resetGroup()
{
    QWidget* t = tabWidget->widget(tabWidget->currentIndex());

    QMap<QString, QWidget*>::const_iterator it;
    QString g;
    for (it = tabs.constBegin(); it!=tabs.constEnd(); it++ ) {
        if (it.value()->parent() == t) {
            g = it.key();
        }
    }

    foreach (VariableWidget* variable, variables) {
        if (variable->getGroup() == g) {
            variable->reset();
        }
    }

    if (g=="Camera") {
        mainWindow->resetCamera(true);
    }

    mainWindow->callRedraw();
}

void VariableEditor::copyGroup()
{
    QWidget* t = tabWidget->widget(tabWidget->currentIndex());

    QMap<QString, QWidget*>::const_iterator it;
    QString g;
    for (it = tabs.constBegin(); it!=tabs.constEnd(); it++ ) {
        if (it.value()->parent() == t) {
            g = it.key();
        }
    }

    QString gs;
    foreach (VariableWidget* variable, variables) {
        if (variable->getGroup() == g) {
            gs += variable->getName() + " = " + variable->getValueAsText() + "\n";
        }
    }

    INFO(tr("Copied ") + g + tr(" settings to clipboard"));
    QClipboard *cb = QApplication::clipboard();
    cb->setText( gs,QClipboard::Clipboard );
}

namespace
{
class ScrollArea : public QScrollArea
{
public:
    ScrollArea(QWidget* child): child(child) {}

    void resizeEvent(QResizeEvent *ev) override
    {
        Q_UNUSED(ev)
        child->setMinimumSize( viewport()->size().width(),child->size().height());
        child->setMaximumSize( viewport()->size().width(),child->size().height());
        child->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        // Seems we have to do this manually...
        QApplication::postEvent(this, new QEvent(QEvent::LayoutRequest));
    }
    QWidget* child;
};
} // namespace

void VariableEditor::childChanged(bool lockedChanged)
{
    emit changed(lockedChanged);
}

void VariableEditor::createGroup(QString g)
{
    if (verbose) {
        qDebug() << "Creating new group -> " + g;
    }

    QWidget *w = new QWidget(this, nullptr);

    w->setLayout(new QVBoxLayout(w));

    w->layout()->setSpacing(10);
    w->layout()->setContentsMargins (10,10,10,10);
    w->setMinimumSize(10,10);

    w->layout()->setSizeConstraint(QLayout::SetMinAndMaxSize);

    auto *sa = new ScrollArea(w);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setWidget(w);
    sa->setContentsMargins (0,0,0,0);
    w->setParent(sa);

    tabWidget->addTab(sa, g);
    tabs[g] = w;

    QWidget* a = new QWidget();
    a->setLayout(new QVBoxLayout(a));
    a->layout()->setSpacing(0);
    a->layout()->setContentsMargins (0,0,0,0);

    QWidget* b = new QWidget();
    b->setLayout(new QHBoxLayout(b));
    b->layout()->setSpacing(0);
    b->layout()->setContentsMargins (0,0,0,0);

    auto *pb = new QPushButton(b);
    pb->setText("Copy group");
    b->layout()->addWidget(pb);
    connect(pb, SIGNAL(clicked()), this, SLOT(copyGroup()));
    pb = new QPushButton(b);
    pb->setText("Lock group");
    b->layout()->addWidget(pb);
    connect(pb, SIGNAL(clicked()), this, SLOT(lockGroup()));
    pb = new QPushButton(b);
    pb->setText("Unlock group");
    b->layout()->addWidget(pb);
    connect(pb, SIGNAL(clicked()), this, SLOT(unlockGroup()));

    a->layout()->addWidget(b);

    QWidget* c = new QWidget();
    c->setLayout(new QHBoxLayout(c));
    c->layout()->setSpacing(0);
    c->layout()->setContentsMargins (0,0,0,0);
    pb = new QPushButton(c);
    pb->setText("Reset group");
    c->layout()->addWidget(pb);
    connect(pb, SIGNAL(clicked()), this, SLOT(resetGroup()));
    a->layout()->addWidget(c);
    auto *verticalSpacer = new QSpacerItem(288, 26, QSizePolicy::Minimum, QSizePolicy::Expanding);
    a->layout()->addItem(verticalSpacer);
    w->layout()->addWidget(a);
    spacers[w] = a;

    update();
}

void VariableEditor::updateTextures(Parser::FragmentSource *fs, FileManager *fileManager)
{
    for (int i = 0; i < variables.count(); i++) {
        variables[i]->updateTextures(fs, fileManager);
    }
}

void VariableEditor::substituteLockedVariables(Parser::FragmentSource *fs)
{
    static QRegExp exp(R"(^\s*uniform\s+(\S+)\s+(\S+)\s*;\s*$)");

    QMap<QString, VariableWidget*> map;
    QStringList names;
    for (int i = 0; i < variables.count(); i++) {
        if (variables[i]->getLockType() == Parser::Locked) {
            map[variables[i]->getName()] = variables[i];
            names.append(variables[i]->getName());
        }
    }

    if (names.count() > 0 && fs->bufferShaderSource == nullptr) {
        INFO(tr("%1 locked variables: %2").arg(map.count()).arg(names.join(",")));
    }

    for (int i = 0; i < fs->source.count(); i++) {
        QString s = fs->source[i];
        if (exp.indexIn(s)!=-1) {
            if (map.contains(exp.cap(2))) {
                QString s;
                if (!useDefines) {
                    s = map[exp.cap(2)]->getLockedSubstitution();
                } else {
                    s = map[exp.cap(2)]->getLockedSubstitution2();
                }

                //                 if(verbose) qDebug() << "Substituted: " + s + " -> " + fs->source[i];
                if (!s.isNull()) {
                    fs->source[i] = s;
                }
            }
        }
    }
}

void VariableEditor::createWidgetFromGuiParameter(Parser::GuiParameter* p) {
//             if(verbose) qDebug() << "Creating: " + p->getName();
    if (dynamic_cast<Parser::FloatParameter *>(p) != nullptr) {
        auto *fp = dynamic_cast<Parser::FloatParameter *>(p);
        QString name = fp->getName();
        FloatWidget *fw = new FloatWidget(currentWidget, this, name, fp->getDefaultValue(), fp->getFrom(), fp->getTo());
        fw->setToolTip(fp->getTooltip());
//                 fw->setStatusTip(fp->getTooltip());
        fw->setGroup(fp->getGroup());
        fw->setDefaultLockType(fp->getLockType());
        fw->setIsDouble(p->isDouble());
        fw->setUpdated(true);
        variables.append(fw);
        currentWidget->layout()->addWidget(fw);
    } else if (dynamic_cast<Parser::Float2Parameter *>(p) != nullptr) {
        auto *f2p = dynamic_cast<Parser::Float2Parameter *>(p);
        QString name = f2p->getName();
        Float2Widget *f2w = new Float2Widget(currentWidget, this, name, f2p->getDefaultValue(), f2p->getFrom(), f2p->getTo());
        f2w->setToolTip(f2p->getTooltip());
//                 f2w->setStatusTip(f2p->getTooltip());
        f2w->setGroup(f2p->getGroup());
        f2w->setDefaultLockType(f2p->getLockType());
        f2w->setIsDouble(p->isDouble());
        f2w->setUpdated(true);
        variables.append(f2w);
        currentWidget->layout()->addWidget(f2w);
    } else if (dynamic_cast<Parser::Float3Parameter *>(p) != nullptr) {
        auto *f3p = dynamic_cast<Parser::Float3Parameter *>(p);
        QString name = f3p->getName();
        Float3Widget *f3w = new Float3Widget(currentWidget, this, name, f3p->getDefaultValue(), f3p->getFrom(), f3p->getTo());
        f3w->setToolTip(f3p->getTooltip());
//                 f3w->setStatusTip(f3p->getTooltip());
        f3w->setGroup(f3p->getGroup());
        f3w->setDefaultLockType(f3p->getLockType());
        f3w->setIsDouble(p->isDouble());
        f3w->setUpdated(true);
        variables.append(f3w);
        currentWidget->layout()->addWidget(f3w);
    } else if (dynamic_cast<Parser::Float4Parameter *>(p) != nullptr) {
        auto *f4p = dynamic_cast<Parser::Float4Parameter *>(p);
        QString name = f4p->getName();
        Float4Widget *f4w = new Float4Widget(currentWidget, this, name, f4p->getDefaultValue(), f4p->getFrom(), f4p->getTo());
        f4w->setToolTip(f4p->getTooltip());
//                 f4w->setStatusTip(f4p->getTooltip());
        f4w->setGroup(f4p->getGroup());
        f4w->setDefaultLockType(f4p->getLockType());
        f4w->setIsDouble(p->isDouble());
        f4w->setUpdated(true);
        variables.append(f4w);
        currentWidget->layout()->addWidget(f4w);
    } else if (dynamic_cast<Parser::IntParameter *>(p) != nullptr) {
        auto *ip = dynamic_cast<Parser::IntParameter *>(p);
        QString name = ip->getName();
        IntWidget *iw = new IntWidget(currentWidget, this, name, ip->getDefaultValue(), ip->getFrom(), ip->getTo());
        iw->setGroup(ip->getGroup());
        iw->setToolTip(ip->getTooltip());
//                 iw->setStatusTip(ip->getTooltip());
        iw->setDefaultLockType(ip->getLockType());
        iw->setUpdated(true);
        variables.append(iw);
        currentWidget->layout()->addWidget(iw);
    } else if (dynamic_cast<Parser::ColorParameter *>(p) != nullptr) {
        auto *cp = dynamic_cast<Parser::ColorParameter *>(p);
        QString name = cp->getName();
        ColorWidget *cw = new ColorWidget(currentWidget, this, name, cp->getDefaultValue());
        cw->setGroup(cp->getGroup());
        cw->setToolTip(cp->getTooltip());
//                 cw->setStatusTip(cp->getTooltip());
        cw->setDefaultLockType(cp->getLockType());
        cw->setIsDouble(p->isDouble());
        cw->setUpdated(true);
        variables.append(cw);
        currentWidget->layout()->addWidget(cw);
    } else if (dynamic_cast<Parser::FloatColorParameter *>(p) != nullptr) {
        auto *cp = dynamic_cast<Parser::FloatColorParameter *>(p);
        QString name = cp->getName();
        FloatColorWidget *cw = new FloatColorWidget( currentWidget, this, name, cp->getDefaultValue(), cp->getFrom(), cp->getTo(), cp->getDefaultColorValue());
        cw->setGroup(cp->getGroup());
        cw->setToolTip(cp->getTooltip());
//                 cw->setStatusTip(cp->getTooltip());
        cw->setDefaultLockType(cp->getLockType());
        cw->setIsDouble(p->isDouble());
        cw->setUpdated(true);
        variables.append(cw);
        currentWidget->layout()->addWidget(cw);
    } else if (dynamic_cast<Parser::BoolParameter *>(p) != nullptr) {
        auto *bp = dynamic_cast<Parser::BoolParameter *>(p);
        QString name = bp->getName();
        BoolWidget *bw = new BoolWidget(currentWidget, this, name, bp->getDefaultValue());
        bw->setToolTip(bp->getTooltip());
//                 bw->setStatusTip(bp->getTooltip());
        bw->setGroup(bp->getGroup());
        bw->setDefaultLockType(bp->getLockType());
        bw->setUpdated(true);
        variables.append(bw);
        currentWidget->layout()->addWidget(bw);
    } else if (dynamic_cast<Parser::SamplerParameter *>(p) != nullptr) {
        auto *sp = dynamic_cast<Parser::SamplerParameter *>(p);
        QString name = sp->getName();
        SamplerWidget *sw = new SamplerWidget(mainWindow->getFileManager(), currentWidget, this, name, sp->getDefaultValue());
        sw->setToolTip(sp->getTooltip());
//                 sw->setStatusTip(sp->getTooltip());
        sw->setGroup(sp->getGroup());
        sw->setDefaultLockType(Parser::AlwaysLocked);
        sw->setUpdated(true);
        variables.append(sw);
        currentWidget->layout()->addWidget(sw);

    } else {
        WARNING(tr("Unsupported parameter"));
    }
}

void VariableEditor::updateFromFragmentSource(Parser::FragmentSource *fs /*, bool *showGUI*/)
{

    if (fs == nullptr) return;

    QVector<Parser::GuiParameter*> ps = fs->params;

    for (int i = 0; i < variables.count(); ) {
        if (variables[i]->isSystemVariable()) {
            variables.remove(i);
            i = 0;
        } else {
            i++;
        }
    }

    for (int i = 0; i < variables.count(); i++) {
        variables[i]->setUpdated(false);
    }

    QMap<QString, bool> tabStillPresent;
    foreach (QString s, tabs.keys()) {
        tabStillPresent[s] = false;
    }

    for (int i = 0; i < ps.count(); i++) {
        QString g = ps[i]->getGroup();
        if (g.isEmpty()) {
            g = "Default";
        }
        if (!tabs.contains(g)) {
            createGroup(g);
        } else {
            if (tabStillPresent.contains(g)) {
                tabStillPresent[g] = true;
            }
        }
        currentWidget = tabs[g];

        bool found = false;
        for (int j = 0; j < variables.count(); j++) {
            QString name = variables[j]->getUniqueName();
            //             if(verbose) qDebug() << "Checking " + name + " -> " +
            //             ps[i]->getUniqueName();
            if (name == ps[i]->getUniqueName()) {
                found = true;
                variables[j]->setUpdated(true);

                variables[j]->setPalette(QApplication::palette(variables[j]));
                variables[j]->setAutoFillBackground(false);

                //                 if(verbose) qDebug() << "Found existing: " +
                //                 variables[j]->getName() + QString(" value:
                //                 %1").arg(variables[j]->getValueAsText());
            }
        }

        // We need to move the spacer to bottom.
        currentWidget->layout()->removeWidget(spacers[currentWidget]);

        if (!found) {
            createWidgetFromGuiParameter(ps[i]);
        }

        // We need to move the spacer to bottom.
        currentWidget->layout()->addWidget(spacers[currentWidget]);

    }

    for (int i = 0; i < variables.count(); ) {
        if (variables[i]->isSystemVariable()) {
            variables.remove(i);
            i = 0;
        } else if (!variables[i]->isUpdated()) {
            delete(variables[i]);
            variables.remove(i);
            i = 0;
        } else {
            i++;
        }
    }

    QMapIterator<QString, bool> it(tabStillPresent);
    while (it.hasNext()) {
        it.next();
        // DBOUT << it.key() << tabs[it.key()]->findChildren<VariableWidget *>(QString(),Qt::FindDirectChildrenOnly).count();
        if (!it.value()) {
            spacers.remove(tabs[it.key()]);
            delete((tabs[it.key()]->parent()));
            tabs.remove(it.key());
        }
    }

    //if (showGUI) (*showGUI) = (variables.count() != 0);
    setPresets(fs->presets);
    updateGeometry();

}

QString VariableEditor::getSettings()
{
    QStringList l;
    for (int i = 0; i < variables.count(); i++) {
        QString name = variables[i]->getName();
        QString val = variables[i]->toSettingsString();
        l.append(name + " = " + val);
    }
    if(saveEasing) {
        QStringList cs = mainWindow->getEngine()->getCurveSettings();
        if(!cs.isEmpty()) {
            for (int i = 0; i < cs.count(); i++) {
                l.append(cs.at(i));
            }
        }
    }
    return l.join("\n");
}

bool VariableEditor::setSettings(QString text)
{
    QStringList l = text.split("\n");
    QMap<QString, QString> maps;

    foreach (QString s, l) {
        s=s.trimmed();
        if (s.startsWith("#")) {
            continue;
        }
        if (s.split(":").count() == 12) { // 12 items in an easing curve setting line
            setEasingCurves(s);
            continue;
        }

        if (s.isEmpty()) {
            continue;
        }

        QStringList l2 = s.split("=");

        if (l2.count()!=2) {
            WARNING(tr("Expected a key value pair, found: ") + s);
            continue;
        }
        QString first = l2[0].trimmed();
        QString second = l2[1].trimmed();
        maps[first] = second;
    }

    bool requiresRecompile = false;

    for (int i = 0; i < variables.count(); i++) {
        if (maps.contains(variables[i]->getName())) {
            requiresRecompile |= variables[i]->fromSettingsString(maps[variables[i]->getName()]);
//             if(verbose) qDebug() << "Found: "+variables[i]->getName();
            maps.remove(variables[i]->getName());
        }
    }

    childChanged(requiresRecompile);

    foreach (QString s, maps.keys()) {
        WARNING(tr("Could not find: ") + s);
    }
    return requiresRecompile;
}

VariableWidget *VariableEditor::getWidgetFromName(QString name)
{
    for (int i = 0; i < variables.count(); i++) {
        if (variables[i]->getName() == name) {
            return variables[i];
        }
    }
    return nullptr;
}

void VariableEditor::updateCamera(CameraControl *c)
{

    QString g = "Camera";
    if (!tabs.contains(g)) {
        createGroup(g);
    }

    QVector<VariableWidget*> added= c->addWidgets(tabs[g], this);

    foreach (VariableWidget* v, added) {
        if (variables.contains(v)) {
            continue;
        }
        v->setGroup(g);
        variables.append(v);
        v->setUpdated(true);
        tabs[g]->layout()->addWidget(v);
    }

    c->connectWidgets(this);
}

bool VariableEditor::eventFilter(QObject *obj, QEvent *ev)
{

    // Pass key events to display widget (to use cursor keys for fine tuning)
    if (ev->type() == QEvent::KeyPress || ev->type() == QEvent::KeyRelease) {
        auto *keyEvent = static_cast<QKeyEvent *>(ev);
        mainWindow->getEngine()->keyPressEvent(keyEvent);
        return true;
    }

    return QWidget::eventFilter(obj, ev);
}

int VariableEditor::addEasingCurve(QString c)
{
    QStringList curveSettings = mainWindow->getEngine()->getCurveSettings();
    int count = curveSettings.count();
    int found = -1;
    QString lookFor = c.split(":").at(0);
    for(int i=0; i<count; i++) {
        if(curveSettings.at(i).startsWith(lookFor)) {
            found = i;
        }
    }
    if(found != -1) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("This variable already has an Easing Curve!"));
        msgBox.setInformativeText(tr("Apply changes? Delete existing? Do nothing?"));
        msgBox.setStandardButtons(QMessageBox::Apply | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Apply);
        int ret = msgBox.exec();
        if(ret == QMessageBox::Apply) {
            INFO(tr("Easing curve %1 replaced by %2").arg(curveSettings.at(found)).arg(c));
            curveSettings.replace(found,c);
            mainWindow->getEngine()->setCurveSettings(curveSettings);
            return found;
        }
        if (ret == QMessageBox::Discard) {
            WARNING(tr("Easing curve %1 removed!").arg(curveSettings.at(found)) );
            curveSettings.removeAt(found);
            mainWindow->getEngine()->setCurveSettings(curveSettings);
            return -1;
        }
        if(ret == QMessageBox::Cancel) {
            INFO(tr("Easing curve canceled!"));
            return -1;
        }
    } else {
        INFO(QString("\n%1").arg(c) );
        WARNING(tr("Insert a \"Range-nnnn-nnnn\" preset to keep this setting!"));
        curveSettings.append(c);
        mainWindow->getEngine()->setCurveSettings(curveSettings);
        mainWindow->documentWasModified();
    }

    setEasingEnabled( !curveSettings.isEmpty() );

    return curveSettings.count();
}

// sets easing curve for currently selected parameter
// tests for existing and warns if found apply|delete|cancel
void VariableEditor::setEasingCurve()
{
    // which combo slider ?
    if (getCurrentComboSlider() != nullptr) {
        // variable to animate
        ComboSlider *cs = getCurrentComboSlider();
        // extract the variable name
        QString variableName = cs->objectName();
        // did we get a variable to animate?
        if( !variableName.isEmpty() ) {
            if (variableName.contains("Eye") || variableName.contains("Target") || variableName.contains("Up")) {
                WARNING(tr("Camera settings handled in KeyFrames!"));
                return;
            }
            // test if already exists
            QStringList check = mainWindow->getEngine()->getCurveSettings();
            int count = check.count();
            int found = -1;
            for(int i=0; i<count; i++) {
                if(check.at(i).startsWith(variableName)) {
                    found = i;
                }
            }

            QEasingCurve::Type curveType = QEasingCurve::Linear; // default curve = 0
            /// NOTE the value 65355.0 passed when found=true because -1,0,1 are valid start params
            /// but 65355.0 is not likely, tells EW we have a curve and not to set defaults
            auto *ew = new EasingWindow(this, cs->getMin(), cs->getMax(),
                                 (found==-1) ? cs->getValue() : 65355.0,
                                 mainWindow->getTimeMax()*mainWindow->renderFPS,
                                 cs->getLoops(), cs->getPong());

            ew->setTitle(tr("Easing Curve for %1").arg(variableName));

            double s,f,p,a,o;
            int ff,lf,l,pp;
            // found a curve so set params in EW
            if(found != -1) {

                QStringList es = mainWindow->getEngine()->getCurveSettings().at(found).split(":");

                s = es.at(3).toDouble();
                f = es.at(4).toDouble();
                ff = es.at(5).toInt();
                lf = es.at(6).toInt();
                p = es.at(7).toDouble();
                a = es.at(8).toDouble();
                o = es.at(9).toDouble();
                l = es.at(10).toInt();
                pp = es.at(11).toInt();

                ew->setCurve(es.at(2).toInt());
                ew->setStartValue(s);
                ew->setFinishValue(f);
                ew->setFirstFrame(ff);
                ew->setLastFrame(lf);
                ew->setPeriod(p);
                ew->setAmplitude(a);
                ew->setOvershoot(o);
                ew->setLoopCount(l);
                ew->setPong(pp);
            }
            // show Easing Curve Window
            if( ew->exec() == 1) { // did user select Ok?
                // get params from EW
                s = ew->getStartValue();
                f = ew->getFinishValue();
                ff = ew->getFirstFrame();
                lf = ew->getLastFrame();
                p = ew->getPeriod();
                a = ew->getAmplitude();
                o = ew->getOvershoot();
                l = ew->getLoopCount();
                pp = ew->getPong();
                // test for validity
                if(s == f) {
                    WARNING(tr("The start value and finish value must be different.") );
                    return;
                }
                if(ff == lf) {
                    WARNING(tr("The first frame and last frame must be different.") );
                    return;
                }
                // selected curve
                curveType = ew->curve.type();
                // get the curve name
                const QMetaObject &mo = QEasingCurve::staticMetaObject;
                QMetaEnum metaEnum = mo.enumerator(mo.indexOfEnumerator("Type"));
                // assemble easing curve string
                QString sc = QString("%1:%2:%3:%4:%5:%6:%7:%8:%9:%10:%11:%12")
                             .arg(variableName)
                             .arg(metaEnum.key(curveType))
                             .arg(curveType)
                             .arg(s)
                             .arg(f)
                             .arg(ff)
                             .arg(lf)
                             .arg(p)
                             .arg(a)
                             .arg(o)
                             .arg(l)
                             .arg(pp);
                // append curve string to list and setup the curve for animating
                if(addEasingCurve(sc) != -1) {
                    setEasingCurves(sc);
                }
            } else {
                INFO(tr("Easing curve canceled!"));
            }
            delete ew; // cleanup!
        } else {
            INFO(tr("No Easing Curve for this parameter."));
        }
    }
}

// adds or replaces existing with provided Easing Curve Settings no dialog
void VariableEditor::setEasingCurves(QString ecset)
{
    QStringList curveSettings = mainWindow->getEngine()->getCurveSettings();
    // variable to animate
    QStringList easingOptions = ecset.split(":");
    QString varName = easingOptions.at(0);
    if (varName.contains("Eye") || varName.contains("Target") || varName.contains("Up")) {
        return;
    }
    // did we get a variable to animate?
    if( !varName.isEmpty() && easingOptions.count() == 12) {
        // which combo slider ?
        auto *cs = findChild<ComboSlider *>(varName);
        if (cs != nullptr) {
            // test if already exists
            int count = curveSettings.count();
            int found = -1;
            for(int i=0; i<count; i++) {
                if(curveSettings.at(i).startsWith(varName)) {
                    found = i;
                }
            }

            auto curveType = (QEasingCurve::Type)easingOptions.at(2).toInt();
            double s,f,p,a,o;
            int ff,lf, l, pp;
            s = easingOptions.at(3).toDouble();
            f = easingOptions.at(4).toDouble();
            ff = easingOptions.at(5).toInt();
            lf = easingOptions.at(6).toInt();
            p = easingOptions.at(7).toDouble();
            a = easingOptions.at(8).toDouble();
            o = easingOptions.at(9).toDouble();
            l = easingOptions.at(10).toInt();
            pp = easingOptions.at(11).toInt();
            // test for validity
            if(s == f) {
                WARNING(tr("The start value and finish value must be different.") );
                return;
            }
            if(ff == lf) {
                WARNING(tr("The first frame and last frame must be different.") );
                return;
            }
            // setup the curve for animating
            if (found != -1) {
                curveSettings.replace(found, ecset);
            } else {
                curveSettings.append(ecset);
            }

            bool isElastic = curveType >= QEasingCurve::InElastic && curveType <= QEasingCurve::OutInElastic;
            bool isBounce = curveType >= QEasingCurve::InBounce && curveType <= QEasingCurve::OutInBounce;
            bool isOverShoot = curveType >= QEasingCurve::InBack && curveType <= QEasingCurve::OutInBack;

            // set the animation attributes for this ComboSlider
            cs->m_anim->setEasingCurve( curveType );

            if (isElastic) {
                cs->m_anim->easingCurve().setPeriod(p);
            }
            if (isElastic || isBounce) {
                cs->m_anim->easingCurve().setAmplitude(a);
            }
            if (isOverShoot) {
                cs->m_anim->easingCurve().setOvershoot(o);
            }

            cs->m_framestart = ff;
            cs->m_framefin = lf;
            cs->m_loops = l;
            cs->m_pong = pp;

            cs->m_anim->setStartValue(s);
            cs->m_anim->setEndValue(f);
            cs->m_anim->setDuration((lf - ff) * ((1.0 / mainWindow->renderFPS) * 1000));
            cs->m_anim->setLoopCount(l);
            cs->m_anim->start();
            cs->m_anim->setPaused(true);

            mainWindow->getEngine()->setCurveSettings(curveSettings);

        }
    } else {
        WARNING(tr("Missing parameter or not applicable. %1").arg(ecset));
    }
}

int VariableEditor::getKeyFrameCount()
{
    int cnt = 0;
    QRegExp rx = QRegExp("(KeyFrame\\.\\d\\d\\d)");
    foreach (QString preset, presets.keys()) {
        if (rx.indexIn(preset) != -1) {
            cnt++;
        }
    }
    return cnt;
}

QStringList VariableEditor::getPresetByName(QString name)
{
    int i = presetComboBox->findText(name, Qt::MatchFixedString);
    if (i>=0) {
        QString presetName = presetComboBox->itemText(i);
        QStringList preset = presets[presetName].split("\n");
        return preset;
    }
    return QStringList(nullptr);
}

int VariableEditor::getCurrentKeyFrame()
{

    QRegExp rx = QRegExp(R"(KeyFrame\.\d\d\d)");
    if(rx.indexIn(presetComboBox->currentText()) != -1) {
        return presetComboBox->currentText().split(".").at(1).toInt();
    }

    return -1;
}

bool VariableEditor::setPreset(QString p)
{
    int item = presetComboBox->findText(p, Qt::MatchFixedString);
    presetComboBox->setCurrentIndex(item);
    return applyPreset();
    /// this bit of fudge sets the current time to keyframe time
//     if(hasKeyFrames())
    //         mainWindow->setTimeSliderValue( getCurrentKeyFrame() *
    //         ((mainWindow->getTimeMax()*mainWindow->renderFPS)/(getKeyFrameCount()-1)));
}
} // namespace GUI
} // namespace Fragmentarium
