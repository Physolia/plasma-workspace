/*
    localegeneratorglibc.cpp
    SPDX-FileCopyrightText: 2022 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "localegeneratorglibc.h"
#include "kcm_regionandlang_debug.h"

LocaleGeneratorGlibc::LocaleGeneratorGlibc(QObject *parent)
    : LocaleGeneratorBase(parent)
    , m_interface(new LocaleGenHelper(QStringLiteral("org.kde.localegenhelper"), QStringLiteral("/LocaleGenHelper"), QDBusConnection::systemBus(), this))
{
    qCDebug(KCM_REGIONANDLANG) << "connect: " << m_interface->isValid();
    connect(m_interface, &LocaleGenHelper::success, this, [this]() {
        Q_EMIT this->needsFont();
    });
    connect(m_interface, &LocaleGenHelper::error, this, &LocaleGeneratorGlibc::userHasToGenerateManually);
}

QString LocaleGeneratorGlibc::supportMode() const
{
    return QStringLiteral("Glibc");
}

void LocaleGeneratorGlibc::localesGenerate(const QStringList &list)
{
    qCDebug(KCM_REGIONANDLANG) << "enable locales: " << list;
    if (!QFile(QStringLiteral("/etc/locale.gen")).exists()) {
        // fedora or centos
        Q_EMIT success();
        return;
    } else {
        qDebug() << "send polkit request";
        m_interface->enableLocales(list);
    }
}
