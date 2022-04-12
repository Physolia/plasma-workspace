#CMakeLists.txt
#SPDX-FileCopyrightText: 2022 Han Young <hanyoung@protonmail.com>
#SPDX-License-Identifier: GPL-2.0-or-later

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_regionandlang\")
if(UBUNTU_PACKAGEKIT OR GLIBC_LOCALE_GEN)
    set(REGION_LANG_GENERATE_LOCALE TRUE)
endif()

################## Find libraries ###################
if(REGION_LANG_GENERATE_LOCALE)
    find_package(PolkitQt5-1)
    set_package_properties(PolkitQt5-1
            PROPERTIES DESCRIPTION "DBus interface wrapper for Polkit"
            PURPOSE "Communicate with localegen helper in region & lang kcm"
            TYPE REQUIRED
    )
endif()

if(UBUNTU_PACKAGEKIT)
    find_package(PackageKitQt${QT_MAJOR_VERSION})
    set_package_properties(PackageKitQt${QT_MAJOR_VERSION}
            PROPERTIES DESCRIPTION "Software Manager integration"
            TYPE OPTIONAL
            PURPOSE "Used to install additional language packages on demand"
            )

endif()

################## Set sources files #################
set(kcm_regionandlang_PART_SRCS
    kcmregionandlang.cpp
    localelistmodel.cpp
    exampleutility.cpp
    optionsmodel.cpp
    languagelistmodel.cpp
    localegenerator.cpp
    localegeneratorbase.cpp
    regionandlangsettings.cpp
)
kconfig_add_kcfg_files(kcm_regionandlang_PART_SRCS regionandlangsettingsbase.kcfgc GENERATE_MOC)

if(GLIBC_LOCALE_GEN)
    set(kcm_regionandlang_PART_SRCS
        ${kcm_regionandlang_PART_SRCS}
        localegeneratorglibc.cpp)
endif()
if(UBUNTU_PACKAGEKIT)
    set(kcm_regionandlang_PART_SRCS
        ${kcm_regionandlang_PART_SRCS}
        localegeneratorubuntu.cpp)
endif()
ecm_qt_declare_logging_category(
    kcm_regionandlang_PART_SRCS
    HEADER kcm_regionandlang_debug.h
    IDENTIFIER KCM_REGIONANDLANG
    CATEGORY_NAME org.kde.kcm_regionandlang
    DESCRIPTION "Region and Language KCM"
    EXPORT kcm_regionandlang
)

################ Build helper and add additional source files #############
if(REGION_LANG_GENERATE_LOCALE)
    add_subdirectory("localegenhelper")
    qt5_generate_dbus_interface(
        ${CMAKE_CURRENT_SOURCE_DIR}/localegenhelper/localegenhelper.h
        org.kde.localegenhelper.xml
        OPTIONS -s -m
    )
    qt5_add_dbus_interface(kcm_regionandlang_PART_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.localegenhelper.xml localegenhelperinterface)
endif()

#################### Declare target #######################
kcoreaddons_add_plugin(kcm_regionandlang SOURCES ${kcm_regionandlang_PART_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings")

ecm_qt_install_logging_categories(
    EXPORT kcm_regionandlang
    DESTINATION "${KDE_INSTALL_LOGGINGCATEGORIESDIR}"
)

################# Link libraries ####################
target_link_libraries(kcm_regionandlang
    Qt::Core
    Qt::DBus
    KF5::I18n
    KF5::KCMUtils
    KF5::QuickAddons)
if (QT_MAJOR_VERSION EQUAL "6")
    target_link_libraries(kcm_formats Qt::Core5Compat) # for QTextCodec
endif()
if(UBUNTU_PACKAGEKIT)
    target_link_libraries(kcm_regionandlang PK::packagekitqt${QT_MAJOR_VERSION})
endif()
########### install files ###############
install(FILES kcm_regionandlang.desktop DESTINATION ${KDE_INSTALL_APPDIR})
kpackage_install_package(package kcm_regionandlang kcms)
