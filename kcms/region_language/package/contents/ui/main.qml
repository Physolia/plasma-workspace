/*
  SPDX-FileCopyrightLabel: 2021 Han Young <hanyoung@protonmail.com>

  SPDX-License-Identifier: LGPL-3.0-or-later
*/
import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kcm 1.2 as KCM
import kcmregionandlang 1.0

KCM.ScrollViewKCM {
    id: root
    implicitHeight: Kirigami.Units.gridUnit * 40
    implicitWidth: Kirigami.Units.gridUnit * 20
    header: ColumnLayout {
        id: messagesLayout

        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            id: takeEffectNextTimeMsg
            Layout.fillWidth: true

            type: Kirigami.MessageType.Information

            text: i18nc("@info", "Changes will take effect the next time you log in.")

            actions: [
                Kirigami.Action {
                    icon.name: "system-reboot"
                    visible: takeEffectNextTimeMsg.visible && !dontShutdownMsg.visible
                    text: i18n("Restart now")
                    onTriggered: {
                        kcm.reboot()
                    }
                }
            ]
        }

        Kirigami.InlineMessage {
            id: dontShutdownMsg
            Layout.fillWidth: true

            type: Kirigami.MessageType.Warning

            text:  i18nc("@info", "Generating locale and language support files; don't turn off the computer yet.")
        }

        Kirigami.InlineMessage {
            id: installFontMsg
            Layout.fillWidth: true

            type: Kirigami.MessageType.Information

            text: i18nc("@info", "Locale and language support files have been generated. Please refer to your distribution's manual to learn how to install relevant fonts packages.")
        }
        Kirigami.InlineMessage {
            id: manualInstallMsg
            Layout.fillWidth: true

            type: Kirigami.MessageType.Error
        }
        Kirigami.InlineMessage {
            id: installSuccessMsg
            Layout.fillWidth: true

            type: Kirigami.MessageType.Positive

            text: i18nc("@info", "Necessary locale and language support files have been installed. It is now safe to turn off the computer.")
        }
    }

    Connections {
        target: kcm
        function onStartGenerateLocale() {
            dontShutdownMsg.visible = true;
            takeEffectNextTimeMsg.visible = true;
        }
        function onRequireInstallFont() {
            dontShutdownMsg.visible = false;
            installFontMsg.visible = true;
        }
        function onUserHasToGenerateManually(reason) {
            manualInstallMsg.text = reason;
            dontShutdownMsg.visible = false;
            manualInstallMsg.visible = true;
        }
        function onGenerateFinished() {
            dontShutdownMsg.visible = false;
            installSuccessMsg.visible = true;
        }
        function onSaved() {
            // return to first page on save action since all messages are here
            while (kcm.depth !== 1) {
                kcm.takeLast();
            }
        }
    }

    view: ListView {
        model: kcm.optionsModel
        delegate: Kirigami.BasicListItem {
            highlighted: false
            hoverEnabled: false

            text: model.name
            subtitle: {
                if (model.page === "lang") {
                    return model.localeName;
                }
                return model.example;
            }
            reserveSpaceForSubtitle: true
            trailing: Item {
                implicitWidth: changeButton.implicitWidth
                QQC2.Button {
                    id: changeButton
                    anchors.centerIn: parent
                    text: i18nc("Button for change the locale used", "Change it…")
                    onClicked: {
                        if (model.page === "lang") {
                            languageSelectPage.active = true;
                            kcm.push(languageSelectPage.item);
                        } else {
                            localeListPage.active = true;
                            kcm.push(localeListPage.item);
                        }
                    }
                }
            }
        }
    }

    Loader {
        id: languageSelectPage
        active: false
        source: "AdvancedLanguageSelectPage.qml"
    }

    Loader {
        id: localeListPage
        active: false
        sourceComponent: KCM.ScrollViewKCM {
            property string setting: "lang"
            property alias filterText: searchField.text
            title: {
                localeListView.currentIndex = -1;
                localeListModel.selectedConfig = setting;
                switch (setting) {
                case "numeric":
                    return i18n("Numbers");
                case "time":
                    return i18n("Time");
                case "currency":
                    return i18n("Currency");
                case "measurement":
                    return i18n("Measurements");
                case "collate":
                    return i18n("Collate");
                }
                return "numeric"; // guard
            }

            LocaleListModel {
                id: localeListModel
                Component.onCompleted: {
                    localeListModel.setLang(kcm.settings.lang);
                }
            }

            Connections {
                target: kcm.settings
                function onLangChanged() {
                    localeListModel.setLang(kcm.settings.lang);
                }
            }

            header: Kirigami.SearchField {
                id: searchField
                onTextChanged: localeListModel.filter = text
            }

            view: ListView {
                id: localeListView
                clip: true
                model: localeListModel.model
                delegate: Kirigami.BasicListItem {
                    icon: model.flag
                    text: model.display
                    subtitle: model.example ? model.example : ''
                    trailing: QQC2.Label {
                        color: Kirigami.Theme.disabledTextColor
                        text: model.localeName
                    }

                    onClicked: {
                        if (model.localeName !== i18n("Default")) {
                            switch (setting) {
                            case "lang":
                                kcm.settings.lang = localeName;
                                break;
                            case "numeric":
                                kcm.settings.numeric = localeName;
                                break;
                            case "time":
                                kcm.settings.time = localeName;
                                break;
                            case "currency":
                                kcm.settings.monetary = localeName;
                                break;
                            case "measurement":
                                kcm.settings.measurement = localeName;
                                break;
                            }
                        } else {
                            kcm.unset(setting);
                        }

                        kcm.takeLast();
                    }
                }
            }
        }
    }
}
