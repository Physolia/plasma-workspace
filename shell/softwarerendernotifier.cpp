/*
    SPDX-FileCopyrightText: 2018 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "softwarerendernotifier.h"

#include <QGuiApplication>
#include <QIcon>
#include <QMenu>
#include <QProcess>
#include <QQuickWindow>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

void SoftwareRendererNotifier::notifyIfRelevant()
{
    if (QQuickWindow::sceneGraphBackend() == QLatin1String("software")) {
        auto group = KSharedConfig::openConfig()->group(QStringLiteral("softwarerenderer"));
        bool neverShow = group.readEntry("neverShow", false);
        if (neverShow) {
            return;
        }
        new SoftwareRendererNotifier(qApp);
    }
}

SoftwareRendererNotifier::SoftwareRendererNotifier(QObject *parent)
    : KStatusNotifierItem(parent)
{
    setTitle(i18n("Software Renderer In Use"));
    setToolTipTitle(i18nc("Tooltip telling user their GL drivers are broken", "Software Renderer In Use"));
    setToolTipSubTitle(i18nc("Tooltip telling user their GL drivers are broken", "Rendering may be degraded"));
    setIconByName(QStringLiteral("video-card-inactive"));
    setStatus(KStatusNotifierItem::Active);
    setStandardActionsEnabled(false);

    connect(this, &KStatusNotifierItem::activateRequested, this, []() {
        QProcess::startDetached(QStringLiteral("kcmshell6"), {QStringLiteral("qtquicksettings")});
    });

    auto menu = new QMenu; // ownership is transferred in setContextMenu
    auto action = new QAction(i18n("Never show again"));
    connect(action, &QAction::triggered, this, [this]() {
        auto group = KSharedConfig::openConfig()->group(QStringLiteral("softwarerenderer"));
        group.writeEntry("neverShow", true);
        deleteLater();
    });
    menu->addAction(action);
    setContextMenu(menu);
}

SoftwareRendererNotifier::~SoftwareRendererNotifier() = default;

#include "moc_softwarerendernotifier.cpp"
