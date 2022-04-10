/*
    optionsmodel.h
    SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once
#include <QAbstractListModel>
#include <array>

#include "regionandlangsettings.h"

using SettingT = RegionAndLangSettings::SettingType;

class RegionAndLangSettings;
class KCMRegionAndLang;
class OptionsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles { Name = Qt::DisplayRole, Subtitle, Example, Page };
    OptionsModel(KCMRegionAndLang *parent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    void handleLangChange();

private:
    enum Formats { Lang = 0, Number, Time, Currency, Measurement };
    QString implicitFormatExampleMsg() const;
    QString getNativeName(const QString &locale) const;

    QString m_numberExample;
    QString m_timeExample;
    QString m_currencyExample;
    QString m_measurementExample;

    std::array<std::pair<QString, QString>, 5> m_staticNames; // title, page

    RegionAndLangSettings *m_settings;
};
