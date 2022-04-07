#! /usr/bin/env bash
$XGETTEXT `find . -path ./plugin/autotests -prune -name \*.js -o -name \*.qml -o -name \*.cpp` -o $podir/plasma_wallpaper_org.kde.image.pot
