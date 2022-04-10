/*
    optionsmodel.cpp
    SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include <QLocale>

#include <KLocalizedString>

#include "exampleutility.cpp"
#include "kcmregionandlang.h"
#include "optionsmodel.h"

OptionsModel::OptionsModel(KCMRegionAndLang *parent)
    : QAbstractListModel(parent)
    , m_settings(parent->settings())
{
    m_staticNames = {{{i18nc("@info:title", "Language"), QStringLiteral("lang")},
                      {i18nc("@info:title", "Numbers"), QStringLiteral("numeric")},
                      {i18nc("@info:title", "Time"), QStringLiteral("time")},
                      {i18nc("@info:title", "Currency"), QStringLiteral("currency")},
                      {i18nc("@info:title", "Measurements"), QStringLiteral("measurement")}}};
    connect(m_settings, &RegionAndLangSettings::langChanged, this, &OptionsModel::handleLangChange);
    connect(m_settings, &RegionAndLangSettings::numericChanged, this, [this] {
        QLocale locale = m_settings->LC_LocaleWithLang(SettingT::Numeric);
        m_numberExample = Utility::numericExample(locale);
        Q_EMIT dataChanged(createIndex(1, 0), createIndex(1, 0), {Subtitle, Example});
    });
    connect(m_settings, &RegionAndLangSettings::timeChanged, this, [this] {
        QLocale locale = m_settings->LC_LocaleWithLang(SettingT::Time);
        m_timeExample = Utility::timeExample(locale);
        Q_EMIT dataChanged(createIndex(2, 0), createIndex(2, 0), {Subtitle, Example});
    });
    connect(m_settings, &RegionAndLangSettings::monetaryChanged, this, [this] {
        QLocale locale = m_settings->LC_LocaleWithLang(SettingT::Monetary);
        m_currencyExample = Utility::monetaryExample(locale);
        Q_EMIT dataChanged(createIndex(3, 0), createIndex(3, 0), {Subtitle, Example});
    });
    connect(m_settings, &RegionAndLangSettings::measurementChanged, this, [this] {
        QLocale locale = m_settings->LC_LocaleWithLang(SettingT::Measurement);
        m_measurementExample = Utility::measurementExample(locale);
        Q_EMIT dataChanged(createIndex(4, 0), createIndex(4, 0), {Subtitle, Example});
    });

    // initialize examples
    m_numberExample = Utility::numericExample(m_settings->LC_LocaleWithLang(SettingT::Numeric));
    m_timeExample = Utility::timeExample(QLocale(m_settings->LC_LocaleWithLang(SettingT::Time)));
    m_measurementExample = Utility::measurementExample(QLocale(m_settings->LC_LocaleWithLang(SettingT::Measurement)));
    m_currencyExample = Utility::monetaryExample(QLocale(m_settings->LC_LocaleWithLang(SettingT::Monetary)));
}

int OptionsModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_staticNames.size();
}

QVariant OptionsModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    if (row < 0 || row >= (int)m_staticNames.size())
        return QVariant();

    switch (role) {
    case Name:
        return m_staticNames[row].first;
    case Subtitle: {
        switch (row) {
        case Lang:
            if (m_settings->defaultLangValue().isEmpty() && m_settings->isDefaultSetting(SettingT::Lang)) {
                // no Lang configured, no $LANG in env
                return i18nc("@info:title, the current setting is system default", "System Default");
            } else if (!m_settings->lang().isEmpty()) {
                // Lang configured and not empty
                return getNativeName(m_settings->lang());
            } else {
                // Lang configured but empty, try to read from $LANG, shouldn't happen on a valid config file
                return getNativeName(m_settings->defaultLangValue());
            }
        case Number:
            if (m_settings->isDefaultSetting(SettingT::Numeric)) {
                return getNativeName(m_settings->numeric());
            }
            break;
        case Time:
            if (m_settings->isDefaultSetting(SettingT::Time)) {
                return getNativeName(m_settings->time());
            }
            break;
        case Currency:
            if (m_settings->isDefaultSetting(SettingT::Monetary)) {
                return getNativeName(m_settings->monetary());
            }
            break;
        case Measurement:
            if (m_settings->isDefaultSetting(SettingT::Measurement)) {
                return getNativeName(m_settings->measurement());
            }
            break;
        }
        return QString(); // implicit locale
    }
    case Example: {
        switch (row) {
        case Lang:
            return QString();
        case Number: {
            QString example = m_numberExample;
            if (m_settings->isDefaultSetting(SettingT::Numeric)) {
                example += implicitFormatExampleMsg();
            }
            return example;
        }
        case Time: {
            QString example = m_timeExample;
            if (m_settings->isDefaultSetting(SettingT::Time)) {
                example += implicitFormatExampleMsg();
            }
            return example;
        }
        case Currency: {
            QString example = m_currencyExample;
            if (m_settings->isDefaultSetting(SettingT::Monetary)) {
                example += implicitFormatExampleMsg();
            }
            return example;
        }
        case Measurement: {
            QString example = m_measurementExample;
            if (m_settings->isDefaultSetting(SettingT::Measurement)) {
                example += implicitFormatExampleMsg();
            }
            return example;
        }
        }
    }
    case Page:
        return m_staticNames[row].second;
    }
    return QVariant();
}

QHash<int, QByteArray> OptionsModel::roleNames() const
{
    return {{Name, "name"}, {Subtitle, "localeName"}, {Example, "example"}, {Page, "page"}};
}

void OptionsModel::handleLangChange()
{
    Q_EMIT dataChanged(createIndex(0, 0), createIndex(0, 0), {Subtitle, Example});
    QLocale lang = QLocale(m_settings->lang());
    if (m_settings->isDefaultSetting(SettingT::Numeric)) {
        m_numberExample = Utility::numericExample(lang);
        Q_EMIT dataChanged(createIndex(1, 0), createIndex(1, 0), {Subtitle, Example});
    }
    if (m_settings->isDefaultSetting(SettingT::Time)) {
        m_timeExample = Utility::timeExample(lang);
        Q_EMIT dataChanged(createIndex(2, 0), createIndex(2, 0), {Subtitle, Example});
    }
    if (m_settings->isDefaultSetting(SettingT::Monetary)) {
        m_currencyExample = Utility::monetaryExample(lang);
        Q_EMIT dataChanged(createIndex(3, 0), createIndex(3, 0), {Subtitle, Example});
    }
    if (m_settings->isDefaultSetting(SettingT::Measurement)) {
        m_measurementExample = Utility::measurementExample(lang);
        Q_EMIT dataChanged(createIndex(4, 0), createIndex(4, 0), {Subtitle, Example});
    }
}

QString OptionsModel::implicitFormatExampleMsg() const
{
    QString locale;

    if (!m_settings->lang().isEmpty()) {
        locale = getNativeName(m_settings->lang());
    } else if (!m_settings->defaultLangValue().isEmpty()) {
        locale = getNativeName(m_settings->defaultLangValue());
    } else {
        locale = i18nc("@info:title, the current setting is system default", "System Default");
    }
    return i18nc("as subtitle, remind user that the format used now is inherited from locale %1", " (Standard format for %1)", locale);
}

QString OptionsModel::getNativeName(const QString &locale) const
{
    return QLocale(locale).nativeLanguageName();
}
