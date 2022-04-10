/*
    localegeneratorbase.h
    SPDX-FileCopyrightText: 2022 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once
#include <QObject>
class LocaleGeneratorBase : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString supportMode READ supportMode CONSTANT)
public:
    LocaleGeneratorBase(QObject *parent = nullptr);
    virtual QString supportMode() const;
    virtual Q_INVOKABLE void localesGenerate(const QStringList &list);
Q_SIGNALS:
    void success();
    void needsFont();
    void userHasToGenerateManually(const QString &reason);
};
