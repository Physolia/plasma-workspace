/*
    localegeneratorubuntu.cpp
    SPDX-FileCopyrightText: 2022 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <PackageKit/Daemon>

#include <QStandardPaths>

#include <KLocalizedString>

#include "kcm_regionandlang_debug.h"
#include "localegeneratorubuntu.h"

LocaleGeneratorUbuntu::LocaleGeneratorUbuntu(QObject *parent)
    : LocaleGeneratorBase(parent)
{
}

QString LocaleGeneratorUbuntu::supportMode() const
{
    return QStringLiteral("Ubuntu");
}

void LocaleGeneratorUbuntu::localesGenerate(const QStringList &list)
{
    ubuntuInstall(list);
    return;
}

void LocaleGeneratorUbuntu::ubuntuInstall(const QStringList &locales)
{
    if (!m_proc) {
        m_proc = new QProcess(this);
    }
    QStringList args;
    args.reserve(locales.size() * 2);
    for (const auto &locale : locales) {
        auto locale_stripped = locale;
        locale_stripped.remove(QStringLiteral(".UTF-8"));
        args.append({QStringLiteral("--language="), locale_stripped});
    }
    QString binaryPath = QStandardPaths::findExecutable(QStringLiteral("check-language-support"));
    if (!binaryPath.isEmpty()) {
        m_proc->setProgram(binaryPath);
        m_proc->setArguments(args);
        connect(m_proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &LocaleGeneratorUbuntu::ubuntuLangCheck);
        m_proc->start();
    } else {
        Q_EMIT userHasToGenerateManually(i18nc("@info:warning", "Can't locate executable `check-language-support`"));
    }
}

void LocaleGeneratorUbuntu::ubuntuLangCheck(int status, QProcess::ExitStatus exit)
{
    Q_UNUSED(exit)
    if (status != 0) {
        // Something wrong with this Ubuntu, don't try further
        Q_EMIT userHasToGenerateManually(
            i18nc("the arg is the output of failed check-language-support call", "check-language-support failed, output: %1", m_proc->readAllStandardOutput()));
        return;
    }
    const QString output = QString::fromUtf8(m_proc->readAllStandardOutput().simplified());
    const QStringList packages = output.split(QLatin1Char(' '));

    auto transaction = PackageKit::Daemon::resolve(packages, PackageKit::Transaction::FilterNotInstalled | PackageKit::Transaction::FilterArch);
    connect(transaction, &PackageKit::Transaction::package, this, [this](PackageKit::Transaction::Info info, const QString &packageID, const QString &summary) {
        Q_UNUSED(info);
        Q_UNUSED(summary);
        m_packageIDs << packageID;
    });
    connect(transaction, &PackageKit::Transaction::errorCode, this, [](PackageKit::Transaction::Error error, const QString &details) {
        qCDebug(KCM_REGIONANDLANG) << "resolve error" << error << details;
    });
    connect(transaction, &PackageKit::Transaction::finished, this, [packages, this](PackageKit::Transaction::Exit status, uint code) {
        qCDebug(KCM_REGIONANDLANG) << "resolve finished" << status << code << m_packageIDs;
        if (m_packageIDs.size() != packages.size()) {
            qCWarning(KCM_REGIONANDLANG) << "Not all missing packages managed to resolve!" << packages << m_packageIDs;
            QString packages_string = packages.join(QChar(';'));
            Q_EMIT userHasToGenerateManually(i18nc("%1 is a list of package names", "Not all missing packages managed to resolve! %1", packages_string));
        }
        auto transaction = PackageKit::Daemon::installPackages(m_packageIDs);
        connect(transaction, &PackageKit::Transaction::errorCode, this, [](PackageKit::Transaction::Error error, const QString &details) {
            qCDebug(KCM_REGIONANDLANG) << "install error:" << error << details;
            Q_EMIT userHasToGenerateManually(i18nc("%1 is a list of package names", "Failed to install package %1", details));
        });
        connect(transaction, &PackageKit::Transaction::finished, this, [this](PackageKit::Transaction::Exit status, uint code) {
            qCDebug(KCM_REGIONANDLANG) << "install finished:" << status << code;
            Q_EMIT success();
        });
    });
}
