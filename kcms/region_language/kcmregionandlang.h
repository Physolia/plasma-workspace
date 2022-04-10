/*
    kcmregionandlang.h
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KConfigGroup>
#include <KQuickAddons/ManagedConfigModule>

class RegionAndLangSettings;
class OptionsModel;
class LocaleGeneratorBase;
class KCMRegionAndLang : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(RegionAndLangSettings *settings READ settings CONSTANT)
    Q_PROPERTY(OptionsModel *optionsModel READ optionsModel CONSTANT)

public:
    explicit KCMRegionAndLang(QObject *parent, const KPluginMetaData &data, const QVariantList &list = QVariantList());
    void save() override;

    RegionAndLangSettings *settings() const;

    OptionsModel *optionsModel() const;
    Q_INVOKABLE QQuickItem *getSubPage(int index) const; // proxy from KQuickAddons to Qml
    Q_INVOKABLE void unset(const QString &setting);
    Q_INVOKABLE static QString toGlibcLocale(const QString &lang);
    Q_INVOKABLE void reboot();
Q_SIGNALS:
    void saved();
    void takeEffectNextTime();
    void startGenerateLocale();
    void generateFinished();
    void requireInstallFont();
    void userHasToGenerateManually(const QString &reason);

private:
    QHash<QString, QString> m_cachedFlags;

    RegionAndLangSettings *m_settings;
    OptionsModel *m_optionsModel;
    LocaleGeneratorBase *m_generator = nullptr;
};
