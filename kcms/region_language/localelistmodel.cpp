/*
 *  localelistmodel.cpp
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>
 *  Copyright 2021 Han Young <hanyoung@protonmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */
#include "localelistmodel.h"
#include "exampleutility.cpp"
#include "kcmregionandlang.h"
#include <KLocalizedString>
#include <QTextCodec>

LocaleListModel::LocaleListModel()
    : m_filterModel(new QSortFilterProxyModel(this))
{
    QList<QLocale> m_locales = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry);
    m_localeData.reserve(m_locales.size() + 1);
    // we use first QString for title in Unset option
    m_localeData.push_back(LocaleData{i18n("Default for System"), QString(), QString(), QString(), i18n("Default"), QLocale()});
    for (auto &locale : m_locales) {
        m_localeData.push_back(LocaleData{locale.nativeLanguageName(),
                                          QLocale::languageToString(locale.language()),
                                          locale.nativeCountryName(),
                                          QLocale::countryToString(locale.country()),
                                          locale.name(),
                                          locale});
    }
    m_filterModel->setSourceModel(this);
    m_filterModel->setFilterRole(FilterRole);
}

int LocaleListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_displayItemSize;
}

QVariant LocaleListModel::data(const QModelIndex &index, int role) const
{
    int dataIndex = index.row();
    const auto &data = m_localeData.at(dataIndex);
    switch (role) {
    case FlagIcon: {
        QString flagCode;
        const QStringList split = data.flagCode.split(QLatin1Char('_'));
        if (split.size() > 1) {
            flagCode = split[1].toLower();
        }
        auto flagIconPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                   QStringLiteral("kf" QT_STRINGIFY(QT_VERSION_MAJOR) "/locale/countries/%1/flag.png").arg(flagCode));
        return flagIconPath;
    }
    case DisplayName: {
        // 0 is unset option, 1 is locale C
        if (dataIndex == 1) {
            return data.flagCode;
        }
        const QString clabel = !data.nativeCountryName.isEmpty() ? data.nativeCountryName : data.englishCountryName;
        QString languageName;
        if (!data.nativeName.isEmpty()) {
            languageName = data.nativeName;
        } else {
            languageName = data.englishName;
        }
        if (dataIndex == 0) {
            return languageName;
        } else {
            return i18n("%1 (%2)", languageName, clabel);
        }
    }
    case LocaleName: {
        QString cvalue = data.flagCode;
        if (!cvalue.contains(QLatin1Char('.')) && cvalue != QLatin1Char('C') && cvalue != i18n("Default")) {
            // explicitly add the encoding,
            // otherwise Qt doesn't accept dead keys and garbles the output as well
            cvalue.append(QLatin1Char('.') + QTextCodec::codecForLocale()->name());
        }
        return cvalue;
    }
    case Example: {
        switch (m_configType) {
        case Lang:
            return QVariant();
        case Numeric:
            return Utility::numericExample(data.locale);
        case Time:
            return Utility::timeExample(data.locale);
        case Currency:
            return Utility::monetaryExample(data.locale);
        case Measurement:
            return Utility::measurementExample(data.locale);
        case Collate:
            return Utility::collateExample(data.locale);
        default:
            return QVariant();
        }
    }
    case FilterRole: {
        return data.englishCountryName + data.nativeCountryName + data.nativeName + data.englishName + data.flagCode;
    }
    }
    return QVariant();
}

QHash<int, QByteArray> LocaleListModel::roleNames() const
{
    return {{LocaleName, "localeName"}, {DisplayName, "display"}, {FlagIcon, "flag"}, {Example, "example"}};
}

const QString &LocaleListModel::filter() const
{
    return m_filter;
}

void LocaleListModel::setFilter(const QString &filter)
{
    m_filterModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
}

QString LocaleListModel::selectedConfig() const
{
    switch (m_configType) {
    case Lang:
        return QStringLiteral("lang");
    case Numeric:
        return QStringLiteral("numeric");
    case Time:
        return QStringLiteral("time");
    case Currency:
        return QStringLiteral("currency");
    case Measurement:
        return QStringLiteral("measurement");
    case Collate:
        return QStringLiteral("collate");
    }
    // won't reach here
    return QString();
}

void LocaleListModel::setSelectedConfig(const QString &config)
{
    if (config == QStringLiteral("lang")) {
        m_configType = Lang;
    } else if (config == QStringLiteral("numeric")) {
        m_configType = Numeric;
    } else if (config == QStringLiteral("time")) {
        m_configType = Time;
    } else if (config == QStringLiteral("measurement")) {
        m_configType = Measurement;
    } else if (config == QStringLiteral("currency")) {
        m_configType = Currency;
    }
    Q_EMIT selectedConfigChanged();
    Q_EMIT dataChanged(createIndex(0, 0), createIndex(rowCount(), 0), QVector<int>(1, Example));
}

void LocaleListModel::setLang(const QString &lang)
{
    QString tmpLang = lang;
    bool isC = false;
    if (lang.isEmpty()) {
        tmpLang = qgetenv("LANG");
        if (tmpLang.isEmpty()) {
            tmpLang = QStringLiteral("C");
            isC = true;
        }
    }

    LocaleData &data = m_localeData.front();
    if (isC) {
        data.nativeName = i18nc("@info:title, meaning the current locale is system default(which is `C`)", "System Default C");
    } else {
        data.nativeName =
            i18nc("@info:title the current locale is the default for %1, %1 is the country name", "Default for %1", QLocale(tmpLang).nativeLanguageName());
    }
    data.locale = QLocale(tmpLang);

    Q_EMIT dataChanged(createIndex(0, 0), createIndex(0, 0));
}

void LocaleListModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
    int updatedSize = std::min(m_displayItemSize + 40, (int)m_localeData.size());

    if (updatedSize > m_displayItemSize) {
        beginInsertRows(QModelIndex(), m_displayItemSize, updatedSize - 1);
        m_displayItemSize = updatedSize;
        endInsertRows();
    }
}
bool LocaleListModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    int maxSize = m_localeData.size();

    return m_displayItemSize < maxSize;
}

QAbstractItemModel *LocaleListModel::model() const
{
    return m_filterModel;
}
