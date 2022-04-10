/*
    regionandlangsettings.h
    SPDX-FileCopyrightText: 2022 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include "regionandlangsettings_p.h"

class RegionAndLangSettings : public RegionAndLangSettings_p
{
    Q_OBJECT
public:
    enum SettingType { Lang, Numeric, Time, Monetary, Measurement };
    RegionAndLangSettings(QObject *parent = nullptr);
    bool isDefaultSetting(SettingType setting) const;
    QString langWithFallback() const;
    QString LC_LocaleWithLang(SettingType setting) const;
};
