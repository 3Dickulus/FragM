#include "Misc.h"
#include <QFileDialog>
#include <QImageWriter>

#include "../Logging/Logging.h"

using namespace SyntopiaCore::Logging;

namespace SyntopiaCore
{
namespace Misc
{

QString GetImageFileName(QWidget *parent, QString label)
{
    QList<QByteArray> a = QImageWriter::supportedImageFormats();
#ifdef USE_OPEN_EXR
    if (!label.contains("screenshot")) {
        a.append("exr");
    }
#endif
    QStringList allowedTypesFilter;
    QStringList allowedTypes;
    for (int i = 0; i < a.count(); i++) {
        allowedTypesFilter.append("*." + a[i]);
        allowedTypes.append(a[i]);
    }
    QString filter = qApp->translate("Misc", "Image Files (") + allowedTypesFilter.join(" ") + ")";

    QString filename = QFileDialog::getSaveFileName(parent, label, QString(), filter);
    if (filename.isEmpty()) {
        INFO(qApp->translate("Misc", "User cancelled save..."));
        return "";
    }

    QString ext = filename.section(".", -1).toLower();
    if (!allowedTypes.contains(ext)) {
        WARNING(qApp->translate("Misc", "Invalid image extension."));
        WARNING(qApp->translate("Misc", "File must be one of the following types: ") + allowedTypes.join(","));
        return "";
    }

    return filename;
}
} // namespace Misc
} // namespace SyntopiaCore
