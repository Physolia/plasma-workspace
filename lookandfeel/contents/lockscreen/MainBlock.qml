/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2

import QtQuick.Layouts 1.1
import QtQuick.Controls 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

import "../components"

SessionManagementScreen {

    property Item mainPasswordBox: _echoOnBox
    property bool lockScreenUiVisible: false

    enum VisibleScreen {
        BlankScreen,
        PromptEchoOnScreen,
        PromptEchoOffScreen,
        InfoMsgScreen,
        ErrorMsgScreen,
        SuccessScreen /* Currently same as BlankScreen, i.e. empty */
    }

    property int visibleScreen: MainBlock.VisibleScreen.BlankScreen
    property Item echoOnMessageLabel: _echoOnMessage
    property Item echoOnBox: _echoOnBox
    property Item echoOffMessageLabel: _echoOffMessage
    property Item echoOffBox: _echoOffBox
    property Item infoMessageLabel: _infoMessage
    property Item errorMessageLabel: _errorMessage

    //the y position that should be ensured visible when the on screen keyboard is visible
    property int visibleBoundary: 0
    //mapFromItem(loginButton, 0, 0).y
    //onHeightChanged: visibleBoundary = mapFromItem(loginButton, 0, 0).y + loginButton.height + PlasmaCore.Units.smallSpacing

    signal promptEchoOnResult(string value)
    signal promptEchoOffResult(string value)

    ColumnLayout {
        Layout.fillWidth: true
        visible: visibleScreen == MainBlock.VisibleScreen.PromptEchoOnScreen
        enabled: visible

        PlasmaComponents3.Label {
            id: _echoOnMessage
            visible: text ? true : false
            font.pointSize: PlasmaCore.Theme.defaultFont.pointSize + 2
            Layout.maximumWidth: units.gridUnit * 16
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true

            PlasmaComponents3.TextField {
                id: _echoOnBox
                font.pointSize: theme.defaultFont.pointSize + 1
                Layout.fillWidth: true

                focus: visible
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText

                // In Qt this is implicitly active based on focus rather than visibility
                // in any other application having a focussed invisible object would be weird
                // but here we are using to wake out of screensaver mode
                // We need to explicitly disable cursor flashing to avoid unnecessary renders
                cursorVisible: visible

                onAccepted: {
                    if (lockScreenUiVisible) {
                        visibleScreen = MainBlock.VisibleScreen.BlankScreen;
                        promptEchoOnResult(echoOnBox.text);
                    }
                }
            }

            PlasmaComponents3.Button {
                id: echoOnButton
                Accessible.name: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Ok")
                Layout.preferredHeight: echoOnBox.implicitHeight
                Layout.preferredWidth: echoOnButton.Layout.preferredHeight

                icon.name: "go-next"

                onClicked: {
                    visibleScreen = MainBlock.VisibleScreen.BlankScreen;
                    promptEchoOnResult(echoOnBox.text);
                }
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        visible: visibleScreen == MainBlock.VisibleScreen.PromptEchoOffScreen
        enabled: visible

        PlasmaComponents3.Label {
            id: _echoOffMessage
            visible: text ? true : false
            font.pointSize: PlasmaCore.Theme.defaultFont.pointSize + 2
            Layout.maximumWidth: units.gridUnit * 16
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true

            PlasmaComponents3.TextField {
                id: _echoOffBox
                font.pointSize: theme.defaultFont.pointSize + 1
                Layout.fillWidth: true

                focus: visible
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData | Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                revealPasswordButtonShown: true

                // In Qt this is implicitly active based on focus rather than visibility
                // in any other application having a focussed invisible object would be weird
                // but here we are using to wake out of screensaver mode
                // We need to explicitly disable cursor flashing to avoid unnecessary renders
                cursorVisible: visible

                onAccepted: {
                    if (lockScreenUiVisible) {
                        visibleScreen = MainBlock.VisibleScreen.BlankScreen;
                        promptEchoOffResult(echoOffBox.text);
                    }
                }
            }

            PlasmaComponents3.Button {
                id: echoOffButton
                Accessible.name: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Ok")
                Layout.preferredHeight: echoOffBox.implicitHeight
                Layout.preferredWidth: echoOffButton.Layout.preferredHeight

                icon.name: "go-next"

                onClicked: {
                    visibleScreen = MainBlock.VisibleScreen.BlankScreen;
                    promptEchoOffResult(echoOffBox.text);
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true
        visible: visibleScreen == MainBlock.VisibleScreen.InfoMsgScreen
        enabled: visible

        PlasmaComponents3.Label {
            id: _infoMessage
            font.pointSize: PlasmaCore.Theme.defaultFont.pointSize + 2
            Layout.maximumWidth: units.gridUnit * 16
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }

    RowLayout {
        Layout.fillWidth: true
        visible: visibleScreen == MainBlock.VisibleScreen.ErrorMsgScreen
        enabled: visible

        PlasmaComponents3.Label {
            id: _errorMessage
            font.pointSize: PlasmaCore.Theme.defaultFont.pointSize + 2
            Layout.maximumWidth: units.gridUnit * 16
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
    }
}
