/*
    regionandlangsettings.cpp
    SPDX-FileCopyrightText: 2022 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "regionandlangsettings.h"
#include "kcm_regionandlang_debug.h"

RegionAndLangSettings::RegionAndLangSettings(QObject *parent)
    : RegionAndLangSettings_p(parent)
{
}

bool RegionAndLangSettings::isDefaultSetting(SettingType setting) const
{
    bool result;
    switch (setting) {
    case Lang:
        result = lang() == defaultLangValue();
        break;
    case Numeric:
        result = numeric() == defaultNumericValue();
        break;
    case Time:
        result = time() == defaultTimeValue();
        break;
    case Monetary:
        result = monetary() == defaultMonetaryValue();
        break;
    case Measurement:
        result = measurement() == defaultMeasurementValue();
        break;
    }
    return result;
}

QString RegionAndLangSettings::langWithFallback() const
{
    QString lang;
    if (isDefaultSetting(Lang) && RegionAndLangSettings::lang().isEmpty()) {
        lang = RegionAndLangSettings::lang();
    } else {
        QString envLang = qEnvironmentVariable("LANG");
        if (!envLang.isEmpty()) {
            lang = envLang;
        } else {
            lang = QLocale::system().name();
        }
    }
    return lang;
}

QString RegionAndLangSettings::LC_LocaleWithLang(SettingType setting) const
{
    QString result;
    bool isDefault = isDefaultSetting(setting);
    switch (setting) {
    case Numeric:
        if (!isDefault) {
            result = numeric();
        }
        break;
    case Time:
        if (!isDefault) {
            result = time();
        }
        break;
    case Monetary:
        if (!isDefault) {
            result = monetary();
        }
        break;
    case Measurement:
        if (!isDefault) {
            result = measurement();
        }
        break;
    case Lang:
        Q_UNREACHABLE();
    }

    if (result.isEmpty()) {
        return langWithFallback();
    } else {
        return result;
    }
}
