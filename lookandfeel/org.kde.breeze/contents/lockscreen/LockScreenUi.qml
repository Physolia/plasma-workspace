/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQml 2.15
import QtQuick 2.8
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.1
import Qt5Compat.GraphicalEffects

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.workspace.components 2.0 as PW
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kscreenlocker 1.0 as ScreenLocker

import org.kde.plasma.private.sessions 2.0
import org.kde.breeze.components

Item {

    id: lockScreenUi
    // If we're using software rendering, draw outlines instead of shadows
    // See https://bugs.kde.org/show_bug.cgi?id=398317
    readonly property bool softwareRendering: GraphicsInfo.api === GraphicsInfo.Software
    property bool hadPrompt: false

    function tryToSwitchUser(canStartSession) {
        if (!defaultToSwitchUser) { // context property
            return
        }
        // If we are in the only session, then going to the session switcher is
        // a pointless extra step; instead create a new session immediately
        if (canStartSession &&
            ((sessionsModel.showNewSessionEntry && sessionsModel.count === 1)  ||
            (!sessionsModel.showNewSessionEntry && sessionsModel.count === 0)) &&
            sessionsModel.canStartNewSession) {
            sessionsModel.startNewSession(true /* lock the screen too */)
        } else {
            mainStack.push(switchSessionPage, {immediate: true})
        }
    }

    Component.onCompleted: Qt.callLater(tryToSwitchUser, true)

    function handleMessage(msg) {
        if (!root.notification) {
            root.notification += msg;
        } else if (root.notification.includes(msg)) {
            root.notificationRepeated();
        } else {
            root.notification += "\n" + msg
        }
    }

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.Complementary

    Connections {
        target: authenticator
        function onFailed(kind) {
            if (kind != 0) { // if this is coming from the noninteractive authenticators
                return;
            }
            const msg = i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Unlocking failed");
            lockScreenUi.handleMessage(msg);
            graceLockTimer.restart();
            notificationRemoveTimer.restart();
            rejectPasswordAnimation.start();
            lockScreenUi.hadPrompt = false;
        }

        function onSucceeded() {
            if (lockScreenUi.hadPrompt) {
                Qt.quit();
            } else {
                mainStack.replace(null, Qt.resolvedUrl("NoPasswordUnlock.qml"), {"userListModel": users}, StackView.Immediate)
                mainStack.forceActiveFocus();
            }
        }

        function onInfoMessageChanged() {
            lockScreenUi.handleMessage(authenticator.infoMessage);
            lockScreenUi.hadPrompt = true;
        }

        function onErrorMessageChanged() {
            lockScreenUi.handleMessage(authenticator.errorMessage);
        }

        function onPromptChanged() {
            root.notification = Qt.binding(() => authenticator.prompt);
            mainBlock.showPassword = true;
            mainBlock.mainPasswordBox.forceActiveFocus();
            lockScreenUi.hadPrompt = true;
        }
        function onPromptForSecretChanged() {
            root.notification = Qt.binding(() => authenticator.promptForSecret);
            mainBlock.showPassword = false;
            mainBlock.mainPasswordBox.forceActiveFocus();
            lockScreenUi.hadPrompt = true;
        }
    }

    SessionManagement {
        id: sessionManagement
    }

    Connections {
        target: sessionManagement
        function onAboutToSuspend() {
            root.clearPassword();
        }
    }

    SessionsModel {
        id: sessionsModel
        showNewSessionEntry: false
    }

    P5Support.DataSource {
        id: keystateSource
        engine: "keystate"
        connectedSources: "Caps Lock"
    }

    Loader {
        id: changeSessionComponent
        active: false
        source: "ChangeSession.qml"
        visible: false
    }

    RejectPasswordAnimation {
        id: rejectPasswordAnimation
        target: mainBlock
    }

    MouseArea {
        id: lockScreenRoot

        property bool blockUI: mainStack.depth > 1 || mainBlock.mainPasswordBox.text.length > 0 || inputPanel.keyboardActive

        x: parent.x
        y: parent.y
        width: parent.width
        height: parent.height
        hoverEnabled: true
        cursorShape: authenticator.state == ScreenLocker.Authenticators.Authenticating ? Qt.ArrowCursor : Qt.BlankCursor
        drag.filterChildren: true
        onPressed: authenticator.startAuthenticating()
        onPositionChanged: authenticator.startAuthenticating()
        onBlockUIChanged: {
            if (blockUI) {
                fadeoutTimer.running = false;
                authenticator.startAuthenticating();
            } else {
                fadeoutTimer.restart();
            }
        }
        Keys.onEscapePressed: {
            // If the escape key is pressed, kscreenlocker will turn off the screen.
            // We do not want to show the password prompt in this case.
            if (authenticator.state == ScreenLocker.Authenticators.Authenticating) {
                authenticator.stopAuthenticating();
                if (inputPanel.keyboardActive) {
                    inputPanel.showHide();
                }
                root.clearPassword();
            }
        }
        Keys.onPressed: event => {
            authenticator.startAuthenticating();
            event.accepted = false;
        }
        Timer {
            id: fadeoutTimer
            interval: 10000
            onTriggered: {
                if (!lockScreenRoot.blockUI) {
                    mainBlock.mainPasswordBox.showPassword = false;
                    authenticator.stopAuthenticating();
                }
            }
        }
        Timer {
            id: notificationRemoveTimer
            interval: 3000
            onTriggered: root.notification = ""
        }
        Timer {
            id: graceLockTimer
            interval: 3000
            onTriggered: {
                root.clearPassword();
                authenticator.startAuthenticating();
            }
        }

        Component.onCompleted: PropertyAnimation { id: launchAnimation; target: lockScreenRoot; property: "opacity"; from: 0; to: 1; duration: Kirigami.Units.veryLongDuration * 2 }

        states: [
            State {
                name: "onOtherSession"
                // for slide out animation
                PropertyChanges { target: lockScreenRoot; y: lockScreenRoot.height }
                // we also change the opacity just to be sure it's not visible even on unexpected screen dimension changes with possible race conditions
                PropertyChanges { target: lockScreenRoot; opacity: 0 }
            }
        ]

        transitions:
            Transition {
            // we only animate switchting to another session, because kscreenlocker doesn't get notified when
            // coming from another session back and so we wouldn't know when to trigger the animation exactly
            from: ""
            to: "onOtherSession"

            PropertyAnimation { id: stateChangeAnimation; properties: "y"; duration: Kirigami.Units.longDuration; easing.type: Easing.InQuad}
            PropertyAnimation { properties: "opacity"; duration: Kirigami.Units.longDuration}

            onRunningChanged: {
                // after the animation has finished switch session: since we only animate the transition TO state "onOtherSession"
                // and not the other way around, we don't have to check the state we transitioned into
                if (/* lockScreenRoot.state == "onOtherSession" && */ !running) {
                    mainStack.currentItem.switchSession()
                }
            }
        }

        WallpaperFader {
            anchors.fill: parent
            state: authenticator.state == ScreenLocker.Authenticators.Authenticating ? "on" : "off"
            source: wallpaper
            mainStack: mainStack
            footer: footer
            clock: clock
        }

        DropShadow {
            id: clockShadow
            anchors.fill: clock
            source: clock
            visible: !softwareRendering
            radius: 6
            samples: 14
            spread: 0.3
            color : "black" // shadows should always be black
            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.veryLongDuration * 2
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Clock {
            id: clock
            property Item shadow: clockShadow
            visible: y > 0
            anchors.horizontalCenter: parent.horizontalCenter
            y: (mainBlock.userList.y + mainStack.y)/2 - height/2
            Layout.alignment: Qt.AlignBaseline
        }

        ListModel {
            id: users

            Component.onCompleted: {
                users.append({
                    name: kscreenlocker_userName,
                    realName: kscreenlocker_userName,
                    icon: kscreenlocker_userImage !== ""
                          ? "file://" + kscreenlocker_userImage.split("/").map(encodeURIComponent).join("/")
                          : "",
                })
            }
        }

        StackView {
            id: mainStack
            anchors {
                left: parent.left
                right: parent.right
            }
            height: lockScreenRoot.height + Kirigami.Units.gridUnit * 3
            focus: true //StackView is an implicit focus scope, so we need to give this focus so the item inside will have it

            // this isn't implicit, otherwise items still get processed for the scenegraph
            visible: opacity > 0

            initialItem: MainBlock {
                id: mainBlock
                lockScreenUiVisible: authenticator.state == ScreenLocker.Authenticators.Authenticating

                showUserList: userList.y + mainStack.y > 0

                enabled: !graceLockTimer.running

                StackView.onStatusChanged: {
                    // prepare for presenting again to the user
                    if (StackView.status === StackView.Activating) {
                        mainPasswordBox.clear();
                        mainPasswordBox.focus = true;
                        root.notification = "";
                    }
                }
                userListModel: users


                notificationMessage: {
                    const parts = [];
                    if (keystateSource.data["Caps Lock"]["Locked"]) {
                        parts.push(i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Caps Lock is on"));
                    }
                    if (root.notification) {
                        parts.push(root.notification);
                    }
                    return parts.join(" • ");
                }

                onPasswordResult: password => {
                    authenticator.respond(password)
                }

                actionItems: [
                    ActionButton {
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Sleep")
                        iconSource: "system-suspend"
                        onClicked: root.suspendToRam()
                        visible: root.suspendToRamSupported
                    },
                    ActionButton {
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Hibernate")
                        iconSource: "system-suspend-hibernate"
                        onClicked: root.suspendToDisk()
                        visible: root.suspendToDiskSupported
                    },
                    ActionButton {
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Switch User")
                        iconSource: "system-switch-user"
                        onClicked: {
                            // If there are no existing sessions to switch to, create a new one instead
                            if (((sessionsModel.showNewSessionEntry && sessionsModel.count === 1) ||
                               (!sessionsModel.showNewSessionEntry && sessionsModel.count === 0)) &&
                               sessionsModel.canSwitchUser) {
                                mainStack.pop({immediate:true})
                                sessionsModel.startNewSession(true /* lock the screen too */)
                                lockScreenRoot.state = ''
                            } else {
                                mainStack.push(switchSessionPage)
                            }
                        }
                        visible: sessionsModel.canStartNewSession && sessionsModel.canSwitchUser
                    }
                ]

                Loader {
                    Layout.topMargin: Kirigami.Units.smallSpacing // some distance to the password field
                    Layout.fillWidth: true
                    Layout.preferredHeight: item ? item.implicitHeight : 0
                    active: config.showMediaControls
                    source: "MediaControls.qml"
                }
            }
        }

        Loader {
            id: inputPanel
            state: "hidden"
            readonly property bool keyboardActive: item ? item.active : false
            Component {
                id: keyboard
                VirtualKeyboard {}
            }
            Component {
                id: keyboardWayland
                VirtualKeyboard_wayland {}
            }
            anchors {
                left: parent.left
                right: parent.right
            }
            function showHide() {
                state = state == "hidden" ? "visible" : "hidden";
            }
            Component.onCompleted: {
                inputPanel.sourceComponent = Qt.platform.pluginName.includes("wayland") ? keyboardWayland : keyboard
            }

            onKeyboardActiveChanged: {
                if (keyboardActive) {
                    state = "visible";
                } else {
                    state = "hidden";
                }
            }

            states: [
                State {
                    name: "visible"
                    PropertyChanges {
                        target: mainStack
                        y: Math.min(0, lockScreenRoot.height - inputPanel.height - mainBlock.visibleBoundary)
                    }
                    PropertyChanges {
                        target: inputPanel
                        y: lockScreenRoot.height - inputPanel.height
                    }
                },
                State {
                    name: "hidden"
                    PropertyChanges {
                        target: mainStack
                        y: 0
                    }
                    PropertyChanges {
                        target: inputPanel
                        y: lockScreenRoot.height - lockScreenRoot.height/4
                    }
                }
            ]
            transitions: [
                Transition {
                    from: "hidden"
                    to: "visible"
                    SequentialAnimation {
                        ScriptAction {
                            script: {
                                inputPanel.item.activated = true;
                                Qt.inputMethod.show();
                            }
                        }
                        ParallelAnimation {
                            NumberAnimation {
                                target: mainStack
                                property: "y"
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                target: inputPanel
                                property: "y"
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.OutQuad
                            }
                        }
                    }
                },
                Transition {
                    from: "visible"
                    to: "hidden"
                    SequentialAnimation {
                        ParallelAnimation {
                            NumberAnimation {
                                target: mainStack
                                property: "y"
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InOutQuad
                            }
                            NumberAnimation {
                                target: inputPanel
                                property: "y"
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InQuad
                            }
                            OpacityAnimator {
                                target: inputPanel
                                duration: Kirigami.Units.longDuration
                                easing.type: Easing.InQuad
                            }
                        }
                        ScriptAction {
                            script: {
                                inputPanel.item.activated = false;
                                Qt.inputMethod.hide();
                            }
                        }
                    }
                }
            ]
        }

        Component {
            id: switchSessionPage
            SessionManagementScreen {
                property var switchSession: finalSwitchSession

                StackView.onStatusChanged: {
                    if (StackView.status == StackView.Activating) {
                        focus = true
                    }
                }

                userListModel: sessionsModel

                // initiating animation of lockscreen for session switch
                function initSwitchSession() {
                    lockScreenRoot.state = 'onOtherSession'
                }

                // initiating session switch and preparing lockscreen for possible return of user
                function finalSwitchSession() {
                    mainStack.pop({immediate:true})
                    if (userListCurrentItem === null) {
                        console.warn("Switching to an undefined user")
                    } else if (userListCurrentItem.vtNumber === undefined) {
                        console.warn("Switching to an undefined VT")
                    }
                    sessionsModel.switchUser(userListCurrentItem.vtNumber)
                    lockScreenRoot.state = ''
                }

                Keys.onLeftPressed: userList.decrementCurrentIndex()
                Keys.onRightPressed: userList.incrementCurrentIndex()
                Keys.onEnterPressed: initSwitchSession()
                Keys.onReturnPressed: initSwitchSession()
                Keys.onEscapePressed: mainStack.pop()

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.gridUnit

                    PlasmaComponents3.Button {
                        Layout.fillWidth: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Switch to This Session")
                        onClicked: initSwitchSession()
                        visible: sessionsModel.count > 0
                    }

                    PlasmaComponents3.Button {
                        Layout.fillWidth: true
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize + 1
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Start New Session")
                        onClicked: {
                            mainStack.pop({immediate:true})
                            sessionsModel.startNewSession(true /* lock the screen too */)
                            lockScreenRoot.state = ''
                        }
                    }
                }


                actionItems: [
                    ActionButton {
                        iconSource: "go-previous"
                        text: i18nd("plasma_lookandfeel_org.kde.lookandfeel","Back")
                        onClicked: mainStack.pop()
                        //Button gets cut off on smaller displays without this.
                        anchors{
                            verticalCenter: parent.top
                        }
                    }
                ]
            }
        }

        Loader {
            active: root.viewVisible
            source: "LockOsd.qml"
            anchors {
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: Kirigami.Units.gridUnit
            }
        }

        // Note: Containment masks stretch clickable area of their buttons to
        // the screen edges, essentially making them adhere to Fitts's law.
        // Due to virtual keyboard button having an icon, buttons may have
        // different heights, so fillHeight is required.
        //
        // Note for contributors: Keep this in sync with SDDM Main.qml footer.
        RowLayout {
            id: footer
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
                margins: Kirigami.Units.smallSpacing
            }
            spacing: Kirigami.Units.smallSpacing

            PlasmaComponents3.ToolButton {
                id: virtualKeyboardButton

                focusPolicy: Qt.TabFocus
                text: i18ndc("plasma_lookandfeel_org.kde.lookandfeel", "Button to show/hide virtual keyboard", "Virtual Keyboard")
                icon.name: inputPanel.keyboardActive ? "input-keyboard-virtual-on" : "input-keyboard-virtual-off"
                onClicked: {
                    // Otherwise the password field loses focus and virtual keyboard
                    // keystrokes get eaten
                    mainBlock.mainPasswordBox.forceActiveFocus();
                    inputPanel.showHide()
                }

                visible: inputPanel.status == Loader.Ready

                Layout.fillHeight: true
                containmentMask: Item {
                    parent: virtualKeyboardButton
                    anchors.fill: parent
                    anchors.leftMargin: -footer.anchors.margins
                    anchors.bottomMargin: -footer.anchors.margins
                }
            }

            PlasmaComponents3.ToolButton {
                id: keyboardButton

                focusPolicy: Qt.TabFocus
                Accessible.description: i18ndc("plasma_lookandfeel_org.kde.lookandfeel", "Button to change keyboard layout", "Switch layout")
                icon.name: "input-keyboard"

                PW.KeyboardLayoutSwitcher {
                    id: keyboardLayoutSwitcher

                    anchors.fill: parent
                    acceptedButtons: Qt.NoButton
                }

                text: keyboardLayoutSwitcher.layoutNames.longName
                onClicked: keyboardLayoutSwitcher.keyboardLayout.switchToNextLayout()

                visible: keyboardLayoutSwitcher.hasMultipleKeyboardLayouts

                Layout.fillHeight: true
                containmentMask: Item {
                    parent: keyboardButton
                    anchors.fill: parent
                    anchors.leftMargin: virtualKeyboardButton.visible ? 0 : -footer.anchors.margins
                    anchors.bottomMargin: -footer.anchors.margins
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Battery {}
        }
    }
}
