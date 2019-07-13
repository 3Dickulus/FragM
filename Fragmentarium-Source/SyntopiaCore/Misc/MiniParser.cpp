#include "MiniParser.h"
#include <qguiapplication.h>

#include "../Logging/Logging.h"

using namespace SyntopiaCore::Logging;

namespace SyntopiaCore
{
namespace Misc
{

MiniParser::MiniParser(QString value, QChar separator)
    : separator(separator), original(value), value(value), paramCount(0) {}

MiniParser &MiniParser::getInt(int &val)
{
    paramCount++;
    QString first = value.section(separator, 0, 0);
    value = value.section(separator, 1);

    if (first.isEmpty()) {
        WARNING(qApp->translate("MiniParser","Expected argument number %1 for %2")
                .arg(paramCount)
                .arg(original));
    }

    bool succes = false;
    int i = first.toInt(&succes);
    if (!succes) {
        WARNING(qApp->translate("MiniParser","Expected argument number %1 to be an integer. Found: %2")
                .arg(paramCount)
                .arg(first));
    }
    val = i;

    return *this;
}

MiniParser &MiniParser::getBool(bool &val)
{
    paramCount++;
    QString first = value.section(separator, 0, 0);
    value = value.section(separator, 1);

    if (first.isEmpty()) {
        WARNING(qApp->translate("MiniParser","Expected argument number %1 for %2")
                .arg(paramCount)
                .arg(original));
    }

    if (first.toLower() == "true") {
        val = true;
    } else if (first.toLower() == "false") {
        val = false;
    } else {
        WARNING(qApp->translate("MiniParser","Expected argument number %1 to be either true or false. Found: %2")
            .arg(paramCount)
            .arg(first));
    }

    return *this;
}

MiniParser &MiniParser::getDouble(double &val)
{
    paramCount++;
    QString first = value.section(separator, 0, 0);
    value = value.section(separator, 1);

    if (first.isEmpty()) {
        WARNING(qApp->translate("MiniParser","Expected argument number %1 for %2")
                .arg(paramCount)
                .arg(original));
    }

    bool succes = false;
    double d = first.toDouble(&succes);
    if (!succes) {
        WARNING(qApp->translate("MiniParser","Expected argument number %1 to be a double. Found: %2")
                .arg(paramCount)
                .arg(first));
    }
    val = d;

    return *this;
}

MiniParser &MiniParser::getFloat(float &val)
{
    paramCount++;
    QString first = value.section(separator, 0, 0);
    value = value.section(separator, 1);

    if (first.isEmpty()) {
        WARNING(qApp->translate("MiniParser","Expected argument number %1 for %2")
                .arg(paramCount)
                .arg(original));
    }

    bool succes = false;
    float d = first.toFloat(&succes);
    if (!succes) {
        WARNING(qApp->translate("MiniParser","Expected argument number %1 to be a float. Found: %2")
                .arg(paramCount)
                .arg(first));
    }
    val = d;

    return *this;
}

} // namespace Misc

} // namespace SyntopiaCore
