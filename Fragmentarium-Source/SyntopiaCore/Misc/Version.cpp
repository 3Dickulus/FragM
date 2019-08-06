#include "Version.h"

#include <utility>

#include "../Logging/Logging.h"
#include <QFile>

using namespace SyntopiaCore::Logging;

namespace SyntopiaCore
{
namespace Misc
{

Version::Version() : revision(0), build(0), codename("")
{
    this->major = 0;
    this->minor = 0;
}

Version::Version(int major, int minor, int revision, int build, QString codename)
    : revision(revision), build(build), codename(std::move(codename))
{
    this->major = major;
    this->minor = minor;
}

QList<Version> Version::GetNewVersions(QString /*url*/) const
{
    return QList<Version>();
}

QString Version::toLongString() const
{
    QString s = QString("%1.%2").arg(major).arg(minor);
    if (revision >= 0) {
        s += QString(".%3").arg(revision);
    }
    if (build >= 0) {
        s += QString(".%4").arg(build);
    }

    if (!codename.isEmpty()) {
        s += " " + codename;
    }

    return s;
}

bool Version::operator<(const Version &rhs) const
{
    if (major == rhs.major) {
        if (minor == rhs.minor) {
            if (revision == rhs.revision) {
                return (build < rhs.build);
            }
            return (revision < rhs.revision);
        }
        return (minor < rhs.minor);
    }
    return (major < rhs.major);
}

bool Version::operator>(const Version &rhs) const
{
    if ((*this) == rhs) {
        return false;
    }
    return !((*this) < rhs);
}

bool Version::operator==(const Version &rhs) const
{
    return ((major == rhs.major) && (minor == rhs.minor) && (revision == rhs.revision) && (build == rhs.build));
}

} // namespace Misc
} // namespace SyntopiaCore
