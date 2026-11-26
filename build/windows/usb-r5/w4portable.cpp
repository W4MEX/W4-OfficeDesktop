// SPDX-FileCopyrightText: 2026 W4
// SPDX-License-Identifier: AGPL-3.0-only
#ifdef _WIN32
#include "w4portable.h"
#include "defines.h"
#include <Windows.h>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTemporaryFile>
#include <vector>
#include <cstdlib>

namespace W4Portable {
QString moduleDirectory()
{
    static const QString directory = [] {
        std::vector<wchar_t> buffer(32768);
        const DWORD n = GetModuleFileNameW(nullptr, buffer.data(), DWORD(buffer.size()));
        if (!n || n >= buffer.size()) return QString();
        return QFileInfo(QString::fromWCharArray(buffer.data(), int(n))).absolutePath();
    }();
    return directory;
}

bool enabled()
{
    static const bool portable = !moduleDirectory().isEmpty() &&
        QFileInfo::exists(moduleDirectory() + "/w4-portable.marker");
    return portable;
}

QString dataRoot()
{
    // The marker belongs in USB/App; data always belongs in its sibling Data.
    // No machine-specific path or environment override is persisted.
    return enabled() ? QDir::cleanPath(QDir(moduleDirectory()).absoluteFilePath("../Data")) : QString();
}

QString profileRoot() { return dataRoot() + "/Profile"; }
QString documentsRoot() { return QDir::cleanPath(dataRoot() + "/../Documents"); }

QString storePath(const QString &path)
{
    if (!enabled() || path.isEmpty()) return path;
    const QString relative = QDir(dataRoot() + "/..").relativeFilePath(path);
    if (!QDir::isAbsolutePath(relative) && relative != ".." && !relative.startsWith("../"))
        return "w4usb:/" + relative;
    return path;
}

QString restorePath(const QString &path)
{
    if (!enabled() || !path.startsWith("w4usb:/")) return path;
    const QString relative = QDir::cleanPath(path.mid(7));
    if (QDir::isAbsolutePath(relative) || relative == ".." || relative.startsWith("../")) return QString();
    return QDir::cleanPath(QDir(dataRoot() + "/..").absoluteFilePath(relative));
}

QString settingsFile(QSettings::Scope scope)
{
    return dataRoot() + "/Settings/" + (scope == QSettings::UserScope ? "user.ini" : "system.ini");
}

QString instanceSuffix()
{
    if (!enabled()) return QString();
    return "-USB-" + QString::fromLatin1(QCryptographicHash::hash(
        QDir::cleanPath(dataRoot()).toCaseFolded().toUtf8(), QCryptographicHash::Sha256).toHex().left(16));
}

QString appUserModelId()
{
    return enabled() ? QStringLiteral("W4.Office.USB.1") : QString::fromUtf8(APP_USER_MODEL_ID);
}

std::unique_ptr<QSettings> createSettings(QSettings::Scope scope)
{
    if (enabled()) {
        auto settings = std::make_unique<QSettings>(settingsFile(scope), QSettings::IniFormat);
        settings->setFallbacksEnabled(false);
        return settings;
    }
    // Keep the installed edition's original scope and fallback behavior.
    if (scope == QSettings::UserScope)
        return std::make_unique<QSettings>(QSettings::NativeFormat, scope, REG_GROUP_KEY, REG_APP_NAME);
    return std::make_unique<QSettings>(scope, REG_GROUP_KEY, REG_APP_NAME);
}

bool initialize(QString *error)
{
    if (!enabled()) return true;
    for (const QString &relative : {"Settings", "Temp", "Local", "Roaming", "Profile", "Cache/QtShaders", "Cache/Qml"}) {
        if (!QDir().mkpath(dataRoot() + "/" + relative)) {
            if (error) *error = "No se puede crear la carpeta Data en la unidad USB.";
            return false;
        }
    }
    if (!QDir().mkpath(documentsRoot())) {
        if (error) *error = "No se puede crear la carpeta Documents en la unidad USB.";
        return false;
    }
    QTemporaryFile probe(dataRoot() + "/Temp/w4-write-check-XXXXXX");
    if (!probe.open() || probe.write("W4", 2) != 2 || !probe.flush()) {
        if (error) *error = "La unidad USB no permite guardar datos. Libera espacio o habilita la escritura.";
        return false;
    }
    const auto setPath = [](const wchar_t *name, const QString &value) {
        const std::wstring native = QDir::toNativeSeparators(value).toStdWString();
        // Update both the CRT environment (Qt) and the Win32 environment (children).
        return _wputenv_s(name, native.c_str()) == 0 && SetEnvironmentVariableW(name, native.c_str()) != 0;
    };
    if (!setPath(L"TEMP", dataRoot() + "/Temp") || !setPath(L"TMP", dataRoot() + "/Temp") ||
        !setPath(L"LOCALAPPDATA", dataRoot() + "/Local") || !setPath(L"APPDATA", dataRoot() + "/Roaming") ||
        !setPath(L"QT_SHADER_CACHE_PATH", dataRoot() + "/Cache/QtShaders") ||
        !setPath(L"QML_DISK_CACHE_PATH", dataRoot() + "/Cache/Qml")) {
        if (error) *error = "No se pudieron preparar las rutas portatiles.";
        return false;
    }
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, dataRoot() + "/Settings");
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, dataRoot() + "/Settings");
    QCoreApplication::setOrganizationName("W4");
    QDir::setCurrent(moduleDirectory());
    return true;
}

bool writeDiagnostics()
{
    if (!enabled()) return false;
    const QJsonObject report {
        {"portable", true}, {"module_directory", moduleDirectory()},
        {"data_root", QDir::cleanPath(dataRoot())}, {"profile_root", QDir::cleanPath(profileRoot())},
        {"settings_user", QDir::cleanPath(settingsFile(QSettings::UserScope))},
        {"settings_system", QDir::cleanPath(settingsFile(QSettings::SystemScope))},
        {"temp", qEnvironmentVariable("TEMP")}, {"local_app_data", qEnvironmentVariable("LOCALAPPDATA")},
        {"roaming_app_data", qEnvironmentVariable("APPDATA")},
        {"instance_suffix", instanceSuffix()}, {"app_user_model_id", appUserModelId()}
    };
    QSaveFile output(dataRoot() + "/diagnostics.json");
    if (!output.open(QIODevice::WriteOnly)) return false;
    const QByteArray bytes = QJsonDocument(report).toJson();
    return output.write(bytes) == bytes.size() && output.commit();
}
}
#endif
