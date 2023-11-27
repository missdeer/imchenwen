/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */
 
import QtQuick
import Qt.labs.settings 1.0 as QSettings
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import com.dfordsoft.imchenwen

Item {
    
    id: window

    // Player settings
    QSettings.Settings {
        id: playerSettings
        category: "player"
        property alias dark_mode: darkCheckBox.checked
        property alias theme: themeComboBox.currentIndex
        property alias url_open_mode: openUrlComboBox.currentIndex
        property alias parser: parserComboBox.currentIndex
        property alias autoplay: autoplayCheckBox.checked
    }
    
    // Video settings
    QSettings.Settings {
        id: videoSettings
        category: "video"
        property alias hwdec: hwdecComboBox.currentIndex
    }
        
    // Network settings
    QSettings.Settings {
        id: networkSettings
        category: "network"
        property alias limit_cache: limitCacheCheckBox.checked
        property alias forward_cache: forwardCacheSpinBox.value
        property alias backward_cache: backwardCacheSpinBox.value
        property alias proxy_type: proxyModeComboBox.currentIndex
        property alias proxy: proxyInput.text
        property alias proxy_only_for_parsing: proxyParsingOnlyCheckBox.checked
        onProxyChanged: accessManager.setupProxy(proxy_type, proxy, proxy_only_for_parsing)
        onProxy_typeChanged: accessManager.setupProxy(proxy_type, proxy, proxy_only_for_parsing)
        onProxy_only_for_parsingChanged: accessManager.setupProxy(proxy_type, proxy, proxy_only_for_parsing)
    }
    
    // Downloader settings
    QSettings.Settings {
        id: downloaderSettings
        category: "downloader"
        property url save_to: Utils.movieLocation()
        property alias max_threads: maxThreadsSpinBox.value
    }

    // Apply skin settings at init
    Component.onCompleted: {
        SkinColor.theme = playerSettings.theme;
        SkinColor.darkModeSet = playerSettings.dark_mode;
    }

    ScrollView {
        anchors.fill: parent
        clip: true
        
        GridLayout {
            columns: 2
            columnSpacing: 10
            
            // Interface
            Label {
                text: qsTr("Interface")
                font.bold: true
                font.pointSize: 16
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Theme") + " (*):" }
            ComboBox {
                id: themeComboBox
                model: [ "Classic", "Material", "Win10" ]
                currentIndex: 1
            }

            CheckBox {
                id: darkCheckBox
                text: qsTr("Dark mode")
                checked: true
                enabled: themeComboBox.currentIndex !== 0
                Layout.columnSpan: 2
                onToggled: SkinColor.darkModeSet = checked
            }
            
            Label { text: qsTr("(*): Restart needed"); Layout.columnSpan: 2 }
            
            // Play
            Label {
                text: qsTr("Play")
                font.bold: true
                font.pointSize: 16
                Layout.columnSpan: 2
                Layout.topMargin: 20
            }

            Label { text: qsTr("Open URL:") }
            ComboBox {
                id: openUrlComboBox
                model: [ qsTr("Question"), qsTr("Play"), qsTr("Download") ]
            }

            Label { text: qsTr("Parse URL with:") }
            ComboBox {
                id: parserComboBox
                model: [ qsTr("Auto"), "lux", "yt-dlp" ]
            }

            CheckBox {
                id: autoplayCheckBox
                text: qsTr("Play videos after being added to playlist")
                checked: true
                Layout.columnSpan: 2
            }

            // Video and audio
            Label {
                text: qsTr("Video")
                font.bold: true
                font.pointSize: 16
                Layout.columnSpan: 2
                Layout.topMargin: 20
            }

            Label { text: qsTr("Decode") + " (*):" }
            ComboBox {
                id: hwdecComboBox
                model: [ "auto", "vaapi", "vdpau", "nvdec" ]
            }
            
            Label { text: qsTr("(*): Restart needed"); Layout.columnSpan: 2 }

            // Cache
            Label {
                text: qsTr("Cache")
                font.bold: true
                font.pointSize: 16
                Layout.columnSpan: 2
                Layout.topMargin: 20
            }

            CheckBox {
                id: limitCacheCheckBox
                text: qsTr("Limit cache size (Restart needed)")
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Forward (MB):"); enabled: limitCacheCheckBox.checked }
            SpinBox {
                id: forwardCacheSpinBox
                from: 1
                to: 200
                value: 50
                implicitWidth: 140
                enabled: limitCacheCheckBox.checked
            }

            Label { text: qsTr("Backward (MB):"); enabled: limitCacheCheckBox.checked }
            SpinBox {
                id: backwardCacheSpinBox
                from: 1
                to: 200
                value: 50
                implicitWidth: 140
                enabled: limitCacheCheckBox.checked
            }

            // Proxy
            Label {
                text: qsTr("Proxy")
                font.bold: true
                font.pointSize: 16
                Layout.columnSpan: 2
                Layout.topMargin: 20
            }            

            Label { text: qsTr("Proxy mode:"); Layout.columnSpan: 2 }
            ComboBox {
                id: proxyModeComboBox
                model: [ "no", "http", "socks5" ]
                Layout.columnSpan: 2
            }
            
            Label { text: qsTr("Proxy:"); Layout.columnSpan: 2 }
            TextField {
                id: proxyInput
                selectByMouse: true
                Layout.columnSpan: 2
                Layout.fillWidth: true
                color: !text.match(/^[A-Za-z0-9\.]+:\d+$/) ? "red" : SkinColor.darkMode ? "white" : "black"
            }
            
            Label {
                text: qsTr("Note: Socks5 is not supported by online videos.")
                wrapMode: Text.WordWrap
                Layout.columnSpan: 2
            }
            
            CheckBox {
                id: proxyParsingOnlyCheckBox
                text: qsTr("Use proxy only for parsing videos")
                Layout.columnSpan: 2
            }

            // Downloader
            Label {
                text: qsTr("Downloader")
                font.bold: true
                font.pointSize: 16
                Layout.columnSpan: 2
                Layout.topMargin: 20
            }
            
            Label { text: qsTr("Maximum number of threads:"); Layout.columnSpan: 2 }
            SpinBox { id: maxThreadsSpinBox; from: 0; to: 100; value: 5; Layout.columnSpan: 2 }
            
            Label { text: qsTr("Save to:"); Layout.columnSpan: 2 }
            Button {
                id: saveToButton
                text: if (Qt.platform.os === "windows" ) {
                    downloaderSettings.save_to.toString().replace("file:///", "").replace(/\//g, '\\')
                } else {
                    downloaderSettings.save_to.toString().replace("file://", "")
                }
                Layout.columnSpan: 2
                onClicked: folderDialog.open()
                // Flatpak version is sandboxed and has no permission to access other folders
                enabled: Utils.environmentVariable("FLATPAK_SANDBOX_DIR") == ""
            }
            FolderDialog {
                id: folderDialog
                title: "Please choose a folder"
                currentFolder: Utils.movieLocation()
                onAccepted: downloaderSettings.save_to = selectedFolder
            }

            // Website settings
            Label {
                text: qsTr("Website settings")
                font.bold: true
                font.pointSize: 16
                Layout.columnSpan: 2
                Layout.topMargin: 20
            }
            Label { text: qsTr("Quality choice:"); Layout.columnSpan: 2 }
            ComboBox {
                id: qualityComboBox
                model: WebsiteSettings.websites
                Layout.columnSpan: 2
                Layout.fillWidth: true
            }
            Button {
                text: qsTr("Remove")
                Layout.columnSpan: 2
                onClicked: {
                    var index = qualityComboBox.currentIndex;
                    if (index != -1) {
                        WebsiteSettings.remove(WebsiteSettings.websites[index]);
                    }
                }
            }
        }
    }
}
