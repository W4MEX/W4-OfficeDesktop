// SPDX-FileCopyrightText: 2026 W4
// SPDX-License-Identifier: AGPL-3.0-only
#pragma once
#ifdef _WIN32
#include <QString>
#include <QSettings>
#include <memory>

namespace W4Portable {
bool enabled();
QString moduleDirectory();
QString dataRoot();
QString profileRoot();
QString documentsRoot();
QString storePath(const QString &path);
QString restorePath(const QString &path);
QString settingsFile(QSettings::Scope scope);
QString instanceSuffix();
QString appUserModelId();
bool initialize(QString *error);
bool writeDiagnostics();
std::unique_ptr<QSettings> createSettings(QSettings::Scope scope);
}
#endif
