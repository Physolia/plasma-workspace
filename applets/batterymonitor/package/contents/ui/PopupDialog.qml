/*
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>
    SPDX-FileCopyrightText: 2013-2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.kquickcontrolsaddons 2.1
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.1 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

PlasmaComponents3.Page {
    id: dialog

    property alias model: batteryList.model
    property bool pluggedIn

    property int remainingTime

    property bool isBrightnessAvailable
    property bool isKeyboardBrightnessAvailable

    property string activeProfile
    property var profiles
    property string inhibitionReason
    property string degradationReason
    // type: [{ Name: string, Icon: string, Profile: string, Reason: string }]
    required property var profileHolds

    signal powermanagementChanged(bool disabled)
    signal activateProfileRequested(string profile)

    header: PlasmaExtras.PlasmoidHeading {
        PowerManagementItem {
            id: pmSwitch
            width: parent.width
            pluggedIn: dialog.pluggedIn
            onDisabledChanged: powermanagementChanged(disabled)
            KeyNavigation.tab: batteryList
            KeyNavigation.backtab: keyboardBrightnessSlider
        }
    }

    FocusScope {
        anchors.fill: parent
        focus: true


        ColumnLayout {
            anchors {
                fill: parent
                margins: PlasmaCore.Units.smallSpacing * 2
            }
            spacing: PlasmaCore.Units.smallSpacing * 2

            BrightnessItem {
                id: brightnessSlider

                icon: "video-display-brightness"
                label: i18n("Display Brightness")
                visible: isBrightnessAvailable
                value: batterymonitor.screenBrightness
                maximumValue: batterymonitor.maximumScreenBrightness
                KeyNavigation.tab: keyboardBrightnessSlider
                KeyNavigation.backtab: batteryList
                stepSize: batterymonitor.maximumScreenBrightness/100

                onMoved: batterymonitor.screenBrightness = value

                // Manually dragging the slider around breaks the binding
                Connections {
                    target: batterymonitor
                    function onScreenBrightnessChanged() {
                        brightnessSlider.value = batterymonitor.screenBrightness
                    }
                }
            }

            BrightnessItem {
                id: keyboardBrightnessSlider

                icon: "input-keyboard-brightness"
                label: i18n("Keyboard Brightness")
                showPercentage: false
                value: batterymonitor.keyboardBrightness
                maximumValue: batterymonitor.maximumKeyboardBrightness
                visible: isKeyboardBrightnessAvailable
                KeyNavigation.tab: pmSwitch
                KeyNavigation.backtab: brightnessSlider

                onMoved: batterymonitor.keyboardBrightness = value

                // Manually dragging the slider around breaks the binding
                Connections {
                    target: batterymonitor
                    function onKeyboardBrightnessChanged() {
                        keyboardBrightnessSlider.value = batterymonitor.keyboardBrightness
                    }
                }
            }

            PowerProfileItem {
                activeProfile: dialog.activeProfile
                inhibitionReason: dialog.inhibitionReason
                visible: dialog.profiles.length > 0
                degradationReason: dialog.degradationReason
                profileHolds: dialog.profileHolds
                onActivateProfileRequested: dialog.activateProfileRequested(profile)
            }

            PlasmaComponents3.ScrollView {
                id: batteryScrollView

                Layout.fillWidth: true
                Layout.fillHeight: true

                // HACK: workaround for https://bugreports.qt.io/browse/QTBUG-83890
                PlasmaComponents3.ScrollBar.horizontal.policy: PlasmaComponents3.ScrollBar.AlwaysOff

                ListView {
                    id: batteryList

                    boundsBehavior: Flickable.StopAtBounds
                    spacing: PlasmaCore.Units.smallSpacing * 2

                    KeyNavigation.tab: brightnessSlider
                    KeyNavigation.backtab: pmSwitch

                    delegate: BatteryItem {
                        width: {
                            const scrollBar = batteryScrollView.PlasmaComponents3.ScrollBar.vertical;
                            const hasScrollBar = scrollBar !== null && scrollBar.visible;
                            // add spacing between an item and the scroll bar
                            return ListView.view.width
                                - (hasScrollBar ? PlasmaCore.Units.smallSpacing * 2 : 0);
                        }
                        battery: model
                    }
                }
            }
        }
    }
}
