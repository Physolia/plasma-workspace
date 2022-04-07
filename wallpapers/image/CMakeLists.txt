add_definitions(-DTRANSLATION_DOMAIN=\"plasma_wallpaper_org.kde.image\")

ecm_qt_declare_logging_category(image_SRCS HEADER debug.h
                                           IDENTIFIER IMAGEWALLPAPER
                                           CATEGORY_NAME kde.wallpapers.image
                                           DEFAULT_SEVERITY Info)

set(plasma-apply-wallpaperimage_SRCS
    plasma-apply-wallpaperimage.cpp
)
add_executable(plasma-apply-wallpaperimage ${plasma-apply-wallpaperimage_SRCS})
target_link_libraries(plasma-apply-wallpaperimage
    Qt::Core
    Qt::DBus
    KF5::I18n
)

install(TARGETS plasma-apply-wallpaperimage DESTINATION ${KDE_INSTALL_BINDIR})

install(FILES wallpaper.knsrc wallpaper-mobile.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

plasma_install_package(imagepackage org.kde.image wallpapers wallpaper)
plasma_install_package(slideshowpackage org.kde.slideshow wallpapers wallpaper)

configure_file(imagepackage/setaswallpaper.desktop.in imagepackage/setaswallpaper.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/imagepackage/setaswallpaper.desktop DESTINATION ${KDE_INSTALL_KSERVICESDIR}/ServiceMenus)

install(DIRECTORY imagepackage/contents/ui DESTINATION ${PLASMA_DATA_INSTALL_DIR}/wallpapers/org.kde.slideshow/contents PATTERN CMakeLists.txt EXCLUDE PATTERN Messages.sh EXCLUDE PATTERN main.qml EXCLUDE PATTERN ConfigGeneral.qml EXCLUDE)

add_subdirectory(plugin)
