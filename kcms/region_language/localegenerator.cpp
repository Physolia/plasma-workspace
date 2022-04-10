/*
    SPDX-FileCopyrightText: 2014 John Layt <john@layt.net>
    SPDX-FileCopyrightText: 2018 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>
    SPDX-FileCopyrightText: 2021 Han Young <hanyoung@protonmail.com>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "localegenerator.h"

#ifdef OS_UBUNTU
#include "localegeneratorubuntu.h"
#elif GLIBC_LOCALE
#include "localegeneratorglibc.h"
#endif

LocaleGeneratorBase *LocaleGenerator::getGenerator()
{
#ifdef OS_UBUNTU
    static auto *singleton = new LocaleGeneratorUbuntu();
#elif GLIBC_LOCALE
    static auto *singleton = new LocaleGeneratorGlibc();
#else
    static auto *singleton = new LocaleGeneratorBase();
#endif
    return singleton;
}
