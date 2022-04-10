/*
    kcmregionandlang.cpp
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <unordered_map>

#include <QCollator>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QQuickItem>

#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

#include "kcmregionandlang.h"
#include "languagelistmodel.h"
#include "localegenerator.h"
#include "localelistmodel.h"
#include "optionsmodel.h"
#include "regionandlangsettings.h"

K_PLUGIN_CLASS_WITH_JSON(KCMRegionAndLang, "kcm_regionandlang.json")

KCMRegionAndLang::KCMRegionAndLang(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, data, args)
    , m_settings(new RegionAndLangSettings(this))
    , m_optionsModel(new OptionsModel(this))
{
    setQuickHelp(i18n("You can configure the formats used for time, dates, money and other numbers here."));

    qmlRegisterAnonymousType<RegionAndLangSettings>("kcmregionandlang", 1);
    qmlRegisterAnonymousType<OptionsModel>("kcmregionandlang", 1);
    qmlRegisterAnonymousType<SelectedLanguageModel>("kcmregionandlang", 1);
    qmlRegisterType<LocaleListModel>("kcmregionandlang", 1, 0, "LocaleListModel");
    qmlRegisterType<LanguageListModel>("kcmregionandlang", 1, 0, "LanguageListModel");
}

void KCMRegionAndLang::save()
{
    using SettingT = RegionAndLangSettings::SettingType;
    // shouldn't have data race issue
    if (!m_generator) {
        m_generator = LocaleGenerator::getGenerator();
        connect(m_generator, &LocaleGeneratorBase::userHasToGenerateManually, this, &KCMRegionAndLang::userHasToGenerateManually);
        connect(m_generator, &LocaleGeneratorBase::success, this, &KCMRegionAndLang::generateFinished);
        connect(m_generator, &LocaleGeneratorBase::needsFont, this, &KCMRegionAndLang::requireInstallFont);
    }

    // assemble full locales in use
    QStringList locales;
    if (settings()->isDefaultSetting(SettingT::Lang)) {
        locales.append(settings()->lang());
    }
    if (settings()->isDefaultSetting(SettingT::Numeric)) {
        locales.append(settings()->numeric());
    }
    if (settings()->isDefaultSetting(SettingT::Time)) {
        locales.append(settings()->time());
    }
    if (settings()->isDefaultSetting(SettingT::Measurement)) {
        locales.append(settings()->measurement());
    }
    if (settings()->isDefaultSetting(SettingT::Monetary)) {
        locales.append(settings()->monetary());
    }
    if (!settings()->language().isEmpty()) {
        QStringList languages = settings()->language().split(QLatin1Char(':'));
        for (const QString &lang : languages) {
            QString glibcLocale = toGlibcLocale(lang);
            if (!glibcLocale.isEmpty()) {
                locales.append(glibcLocale);
            }
        }
    }

    if (!locales.isEmpty()) {
        Q_EMIT startGenerateLocale();
        m_generator->localesGenerate(locales);
    }
    ManagedConfigModule::save();
    Q_EMIT takeEffectNextTime();
    Q_EMIT saved();
}

RegionAndLangSettings *KCMRegionAndLang::settings() const
{
    return m_settings;
}

OptionsModel *KCMRegionAndLang::optionsModel() const
{
    return m_optionsModel;
}

QQuickItem *KCMRegionAndLang::getSubPage(int index) const
{
    return subPage(index);
}

void KCMRegionAndLang::unset(const QString &setting)
{
    const char *entry;
    if (setting == QStringLiteral("lang")) {
        entry = "LANG";
        settings()->setLang(settings()->defaultLangValue());
    } else if (setting == QStringLiteral("numeric")) {
        entry = "LC_NUMERIC";
        settings()->setNumeric(settings()->defaultNumericValue());
    } else if (setting == QStringLiteral("time")) {
        entry = "LC_TIME";
        settings()->setTime(settings()->defaultTimeValue());
    } else if (setting == QStringLiteral("measurement")) {
        entry = "LC_MEASUREMENT";
        settings()->setMeasurement(settings()->defaultMeasurementValue());
    } else {
        entry = "LC_MONETARY";
        settings()->setMonetary(settings()->defaultMonetaryValue());
    }
    settings()->config()->group(QStringLiteral("Formats")).deleteEntry(entry);
}

void KCMRegionAndLang::reboot()
{
    auto method = QDBusMessage::createMethodCall(QStringLiteral("org.kde.LogoutPrompt"),
                                                 QStringLiteral("/LogoutPrompt"),
                                                 QStringLiteral("org.kde.LogoutPrompt"),
                                                 QStringLiteral("promptReboot"));
    QDBusConnection::sessionBus().asyncCall(method);
}

QString KCMRegionAndLang::toGlibcLocale(const QString &lang)
{
    static std::unordered_map<QString, QString> KDELocaleToGlibcLocale = {{QStringLiteral("sv"), QStringLiteral("sv_SE.UTF-8")},
                                                                          {QStringLiteral("sk"), QStringLiteral("sk_SK.UTF-8")},
                                                                          {QStringLiteral("hu"), QStringLiteral("hu_HU.UTF-8")},
                                                                          {QStringLiteral("sr@ijekavianlatin"), QStringLiteral("sr_RS@latin.UTF-8")},
                                                                          {QStringLiteral("sr@ijekavian"), QStringLiteral("sr_ME.UTF-8")},
                                                                          {QStringLiteral("az"), QStringLiteral("az_AZ.UTF-8")},
                                                                          {QStringLiteral("ml"), QStringLiteral("ml_IN.UTF-8")},
                                                                          {QStringLiteral("pt"), QStringLiteral("pt_PT.UTF-8")},
                                                                          {QStringLiteral("ar"), QStringLiteral("ar_AE.UTF-8")},
                                                                          {QStringLiteral("es"), QStringLiteral("es_ES.UTF-8")},
                                                                          {QStringLiteral("da"), QStringLiteral("da_DK.UTF-8")},
                                                                          {QStringLiteral("pa"), QStringLiteral("pa_IN.UTF-8")},
                                                                          {QStringLiteral("bs"), QStringLiteral("bs_BA.UTF-8")},
                                                                          {QStringLiteral("et"), QStringLiteral("et.EE.UTF-8")},
                                                                          {QStringLiteral("lt"), QStringLiteral("lt_LT.UTF-8")},
                                                                          {QStringLiteral("sr@latin"), QStringLiteral("sr_RS@latin.UTF-8")},
                                                                          {QStringLiteral("cs"), QStringLiteral("cs_CZ.UTF-8")},
                                                                          {QStringLiteral("ro"), QStringLiteral("ro_RO.UTF-8")},
                                                                          {QStringLiteral("fi"), QStringLiteral("fi_FI.UTF-8")},
                                                                          {QStringLiteral("eu"), QStringLiteral("eu_ES.UTF-8")},
                                                                          {QStringLiteral("tr"), QStringLiteral("tr_TR.UTF-8")},
                                                                          {QStringLiteral("en_US"), QStringLiteral("en_US.UTF-8")},
                                                                          {QStringLiteral("pl"), QStringLiteral("pl_PL.UTF-8")},
                                                                          {QStringLiteral("uk"), QStringLiteral("uk_UA.UTF-8")},
                                                                          {QStringLiteral("ja"), QStringLiteral("ja_JP.UTF-8")},
                                                                          {QStringLiteral("sl"), QStringLiteral("sl_SI.UTF-8")},
                                                                          {QStringLiteral("nds"), QStringLiteral("nds_DE.UTF-8")},
                                                                          {QStringLiteral("ca@valencia"), QStringLiteral("ca_ES@valencia.UTF-8")},
                                                                          {QStringLiteral("pt_BR"), QStringLiteral("pt_BR.UTF-8")},
                                                                          {QStringLiteral("vi"), QStringLiteral("vi_VN.UTF-8")},
                                                                          {QStringLiteral("hi"), QStringLiteral("hi_IN.UTF-8")},
                                                                          {QStringLiteral("nl"), QStringLiteral("nl_NL.UTF-8")},
                                                                          {QStringLiteral("ru"), QStringLiteral("ru_RU.UTF-8")},
                                                                          {QStringLiteral("ca"), QStringLiteral("ca_ES.UTF-8")},
                                                                          {QStringLiteral("fr"), QStringLiteral("fr_FR.UTF-8")},
                                                                          {QStringLiteral("nn"), QStringLiteral("nn_NO.UTF-8")},
                                                                          {QStringLiteral("nb"), QStringLiteral("nb_NO.UTF-8")},
                                                                          {QStringLiteral("he"), QStringLiteral("he_IL.UTF-8")},
                                                                          {QStringLiteral("ia"), QStringLiteral("ia_FR.UTF-8")},
                                                                          {QStringLiteral("ta"), QStringLiteral("ta_IN.UTF-8")},
                                                                          {QStringLiteral("en_GB"), QStringLiteral("en_GB.UTF-8")},
                                                                          {QStringLiteral("ko"), QStringLiteral("ko_KR.UTF-8")},
                                                                          {QStringLiteral("de"), QStringLiteral("de_DE.UTF-8")},
                                                                          {QStringLiteral("it"), QStringLiteral("it_IT.UTF-8")},
                                                                          {QStringLiteral("gl"), QStringLiteral("gl_ES.UTF-8")},
                                                                          {QStringLiteral("zh_TW"), QStringLiteral("zh_TW.UTF-8")},
                                                                          {QStringLiteral("id"), QStringLiteral("id_ID.UTF-8")},
                                                                          {QStringLiteral("lv"), QStringLiteral("lv_LV.UTF-8")},
                                                                          {QStringLiteral("sr"), QStringLiteral("sr_RS.UTF-8")},
                                                                          {QStringLiteral("tg"), QStringLiteral("tg_TJ.UTF-8")},
                                                                          {QStringLiteral("el"), QStringLiteral("el_GR.UTF-8")},
                                                                          {QStringLiteral("zh_CN"), QStringLiteral("zh_CN.UTF-8")},
                                                                          {QStringLiteral("C"), QStringLiteral("C")}};
    return KDELocaleToGlibcLocale[lang];
}
#include "kcmregionandlang.moc"
