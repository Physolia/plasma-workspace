/*
 *  localelistmodel.h
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
#ifndef LOCALELISTMODEL_H
#define LOCALELISTMODEL_H

#include <QAbstractListModel>
#include <QLocale>
#include <QSortFilterProxyModel>

struct LocaleData {
    QString nativeName;
    QString englishName;
    QString nativeCountryName;
    QString englishCountryName;
    QString flagCode;
    QLocale locale;
};

class LocaleListModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)
    Q_PROPERTY(QString selectedConfig READ selectedConfig WRITE setSelectedConfig NOTIFY selectedConfigChanged)
    Q_PROPERTY(QAbstractItemModel *model READ model CONSTANT)
public:
    enum RoleName { DisplayName = Qt::DisplayRole, LocaleName, FlagIcon, Example, FilterRole };
    enum ConfigType { Lang, Numeric, Time, Currency, Measurement, Collate };
    LocaleListModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    void fetchMore(const QModelIndex &parent) override;
    bool canFetchMore(const QModelIndex &parent) const override;

    const QString &filter() const;
    void setFilter(const QString &filter);
    QAbstractItemModel *model() const;
    QString selectedConfig() const;
    void setSelectedConfig(const QString &config);
    Q_INVOKABLE void setLang(const QString &lang);

Q_SIGNALS:
    void filterChanged();
    void selectedConfigChanged();

private:
    void getExample();

    QSortFilterProxyModel *m_filterModel = nullptr;

    int m_displayItemSize = 40;
    QString m_filter;
    std::vector<LocaleData> m_localeData;
    ConfigType m_configType = Lang;
};

#endif // LOCALELISTMODEL_H
