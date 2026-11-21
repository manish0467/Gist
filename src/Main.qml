import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 1100
    height: 720
    minimumWidth: 800
    minimumHeight: 520
    visible: true
    title: qsTr("Gist — Fast AI-Augmented Email")
    color: "#121316"

    Material.theme: Material.Dark
    Material.accent: theme.accent
    Material.roundedScale: theme.radius

    property bool helpVisible: false
    property bool composeVisible: false
    property bool addAccountVisible: false

    Shortcut { sequence: "?"; context: Qt.ApplicationShortcut; onActivated: win.helpVisible = !win.helpVisible }
    Shortcut { sequence: "Q"; context: Qt.ApplicationShortcut; onActivated: Qt.quit() }
    Shortcut { sequence: "J"; context: Qt.ApplicationShortcut; onActivated: backend.nextThread() }
    Shortcut { sequence: "K"; context: Qt.ApplicationShortcut; onActivated: backend.previousThread() }
    Shortcut { sequence: "S"; context: Qt.ApplicationShortcut; onActivated: backend.primaryAction() }
    Shortcut { sequence: "Space"; context: Qt.ApplicationShortcut; onActivated: backend.primaryAction() }
    Shortcut { sequence: "E"; context: Qt.ApplicationShortcut; onActivated: backend.archiveCurrent() }
    Shortcut { sequence: "#"; context: Qt.ApplicationShortcut; onActivated: backend.deleteCurrent() }
    Shortcut { sequence: "/"; context: Qt.ApplicationShortcut; onActivated: searchInput.forceActiveFocus() }
    Shortcut { sequence: "C"; context: Qt.ApplicationShortcut; onActivated: win.composeVisible = true }
    Shortcut { sequence: "R"; context: Qt.ApplicationShortcut; onActivated: backend.generateReply("normal") }

    Item {
        id: rootLayout
        anchors.fill: parent
        LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
        LayoutMirroring.childrenInherit: true

        RowLayout {
            anchors.fill: parent
            spacing: 0

            // -------------------------------------------------------------
            // PANE 1: Left Navigation Sidebar
            // -------------------------------------------------------------
            Rectangle {
                Layout.preferredWidth: 220
                Layout.fillHeight: true
                color: "#17181c"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 14

                    // App Title & Brand
                    RowLayout {
                        spacing: 8
                        Rectangle {
                            width: 28
                            height: 28
                            radius: 7
                            color: theme.accent
                            Text {
                                anchors.centerIn: parent
                                text: "G"
                                font.bold: true
                                font.pixelSize: 16
                                color: theme.accentForeground
                            }
                        }
                        Text {
                            text: "Gist"
                            font.bold: true
                            font.pixelSize: 18
                            color: "#ffffff"
                        }
                        Item { Layout.fillWidth: true }
                        ToolButton {
                            text: "⚙"
                            font.pixelSize: 15
                            ToolTip.visible: hovered
                            ToolTip.text: qsTr("Add IMAP Account")
                            onClicked: win.addAccountVisible = true
                        }
                    }

                    // Compose Button
                    Button {
                        Layout.fillWidth: true
                        text: qsTr("＋ Compose")
                        highlighted: true
                        Material.background: theme.accent
                        Material.foreground: theme.accentForeground
                        onClicked: win.composeVisible = true
                    }

                    // Navigation Folders
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Repeater {
                            model: [
                                { "id": "inbox", "name": qsTr("Inbox"), "icon": "📥", "count": backend.unreadCount },
                                { "id": "starred", "name": qsTr("Starred"), "icon": "⭐", "count": backend.starredCount },
                                { "id": "sent", "name": qsTr("Sent"), "icon": "📤", "count": 0 },
                                { "id": "archive", "name": qsTr("Archive"), "icon": "🗄️", "count": 0 },
                                { "id": "trash", "name": qsTr("Trash"), "icon": "🗑️", "count": 0 }
                            ]

                            delegate: Rectangle {
                                Layout.fillWidth: true
                                height: 36
                                radius: 6
                                color: backend.activeFolder === modelData.id ? Qt.rgba(255, 255, 255, 0.08) : "transparent"

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 10
                                    spacing: 10

                                    Text {
                                        text: modelData.icon
                                        font.pixelSize: 14
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.name
                                        font.pixelSize: 13
                                        font.weight: backend.activeFolder === modelData.id ? Font.DemiBold : Font.Normal
                                        color: backend.activeFolder === modelData.id ? "#ffffff" : "#a0a4b0"
                                    }

                                    Rectangle {
                                        visible: modelData.count > 0
                                        width: Math.max(18, countText.contentWidth + 8)
                                        height: 18
                                        radius: 9
                                        color: theme.accent

                                        Text {
                                            id: countText
                                            anchors.centerIn: parent
                                            text: modelData.count
                                            font.pixelSize: 11
                                            font.bold: true
                                            color: theme.accentForeground
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: backend.selectFolder(modelData.id)
                                }
                            }
                        }
                    }

                    Item { Layout.fillHeight: true }

                    // Sync & Status Indicator
                    Rectangle {
                        Layout.fillWidth: true
                        height: 38
                        radius: 6
                        color: Qt.rgba(255, 255, 255, 0.04)
                        border.color: backend.isSyncing ? theme.accent : "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 8
                            Text {
                                text: backend.isSyncing ? "🔄" : "⚡"
                                font.pixelSize: 12
                            }
                            Text {
                                Layout.fillWidth: true
                                text: backend.statusMessage || qsTr("Rust IMAP Core Idle")
                                font.pixelSize: 10
                                color: "#8a8f9d"
                                elide: Text.ElideRight
                            }
                        }
                    }

                    // Keyboard Hint
                    Rectangle {
                        Layout.fillWidth: true
                        height: 32
                        radius: 6
                        color: Qt.rgba(255, 255, 255, 0.04)

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 6
                            Text { text: "Press"; font.pixelSize: 11; color: "#777c88" }
                            Rectangle {
                                width: 16; height: 16; radius: 3; color: "#2c2e35"
                                Text { anchors.centerIn: parent; text: "?"; font.pixelSize: 10; color: "#ffffff" }
                            }
                            Text { text: "for shortcuts"; font.pixelSize: 11; color: "#777c88" }
                        }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: win.helpVisible = true
                        }
                    }
                }
            }

            // Divider 1
            Rectangle { width: 1; Layout.fillHeight: true; color: "#24262c" }

            // -------------------------------------------------------------
            // PANE 2: Email Threads List
            // -------------------------------------------------------------
            Rectangle {
                Layout.preferredWidth: 340
                Layout.fillHeight: true
                color: "#1a1b20"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Search Header
                    Rectangle {
                        Layout.fillWidth: true
                        height: 56
                        color: "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8

                            TextField {
                                id: searchInput
                                Layout.fillWidth: true
                                placeholderText: qsTr("Search emails (/)...")
                                font.pixelSize: 13
                                color: "#ffffff"
                                selectByMouse: true
                                background: Rectangle {
                                    radius: 6
                                    color: "#24262d"
                                    border.color: searchInput.activeFocus ? theme.accent : "#2f323c"
                                }
                                onTextChanged: backend.searchQuery = text
                            }

                            ToolButton {
                                text: "↻"
                                font.pixelSize: 15
                                onClicked: backend.refresh()
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#24262c" }

                    // Thread list
                    ListView {
                        id: threadListView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: backend.threads
                        currentIndex: backend.selectedThreadIndex

                        delegate: Rectangle {
                            id: threadDelegate
                            width: threadListView.width
                            height: 84
                            color: ListView.isCurrentItem ? Qt.rgba(255, 255, 255, 0.08) :
                                   delegateMouse.containsMouse ? Qt.rgba(255, 255, 255, 0.03) : "transparent"

                            Rectangle {
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.bottom: parent.bottom
                                width: 3
                                color: theme.accent
                                visible: ListView.isCurrentItem
                            }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                anchors.leftMargin: 14
                                spacing: 3

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Rectangle {
                                        width: 8
                                        height: 8
                                        radius: 4
                                        color: theme.accent
                                        visible: modelData.isUnread
                                    }

                                    Text {
                                        Layout.fillWidth: true
                                        text: modelData.senderName
                                        font.bold: modelData.isUnread
                                        font.pixelSize: 13
                                        color: "#ffffff"
                                        elide: Text.ElideRight
                                    }

                                    Text {
                                        text: modelData.date
                                        font.pixelSize: 11
                                        color: "#7e8391"
                                    }
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.subject
                                    font.bold: modelData.isUnread
                                    font.pixelSize: 12
                                    color: modelData.isUnread ? "#f0f2f5" : "#c4c8d4"
                                    elide: Text.ElideRight
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: modelData.snippet
                                    font.pixelSize: 11
                                    color: "#7e8391"
                                    elide: Text.ElideRight
                                }
                            }

                            MouseArea {
                                id: delegateMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: backend.selectThread(index)
                            }
                        }

                        ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
                    }
                }
            }

            // Divider 2
            Rectangle { width: 1; Layout.fillHeight: true; color: "#24262c" }

            // -------------------------------------------------------------
            // PANE 3: Reading Pane & Gist AI Drawer
            // -------------------------------------------------------------
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#131417"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    // Action Toolbar
                    Rectangle {
                        Layout.fillWidth: true
                        height: 56
                        color: "#18191e"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 16
                            anchors.rightMargin: 16
                            spacing: 10

                            Button {
                                text: qsTr("⚡ Get Gist (Space)")
                                highlighted: true
                                Material.background: theme.accent
                                Material.foreground: theme.accentForeground
                                onClicked: backend.primaryAction()
                            }

                            Button {
                                text: qsTr("Reply")
                                onClicked: backend.generateReply("normal")
                            }

                            Button {
                                text: qsTr("Archive (E)")
                                onClicked: backend.archiveCurrent()
                            }

                            Button {
                                text: qsTr("Delete (#)")
                                onClicked: backend.deleteCurrent()
                            }

                            Item { Layout.fillWidth: true }

                            ToolButton {
                                text: backend.currentThread.isStarred ? "★" : "☆"
                                font.pixelSize: 16
                                onClicked: backend.toggleStarCurrent()
                            }

                            ToolButton {
                                text: backend.aiDrawerOpen ? "Hide AI ❯" : "AI Drawer ❮"
                                onClicked: backend.aiDrawerOpen = !backend.aiDrawerOpen
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; height: 1; color: "#24262c" }

                    // Content & Drawer Split
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 0

                        // Main Email Reading View
                        Flickable {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            contentWidth: width
                            contentHeight: contentCol.height + 40
                            clip: true

                            ColumnLayout {
                                id: contentCol
                                width: parent.width - 48
                                anchors.horizontalCenter: parent.horizontalCenter
                                y: 24
                                spacing: 18

                                // Subject Title
                                Text {
                                    Layout.fillWidth: true
                                    text: backend.currentThread.subject || qsTr("No conversation selected")
                                    font.pixelSize: 22
                                    font.bold: true
                                    color: "#ffffff"
                                    wrapMode: Text.Wrap
                                }

                                // Sender Metadata Card
                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 52
                                    radius: 8
                                    color: "#1b1c22"

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 10
                                        spacing: 12

                                        Rectangle {
                                            width: 32
                                            height: 32
                                            radius: 16
                                            color: theme.accent
                                            Text {
                                                anchors.centerIn: parent
                                                text: (backend.currentThread.senderName || "?").charAt(0)
                                                font.bold: true
                                                font.pixelSize: 15
                                                color: theme.accentForeground
                                            }
                                        }

                                        ColumnLayout {
                                            Layout.fillWidth: true
                                            spacing: 2
                                            Text {
                                                text: backend.currentThread.senderName || ""
                                                font.bold: true
                                                font.pixelSize: 13
                                                color: "#ffffff"
                                            }
                                            Text {
                                                text: "<" + (backend.currentThread.senderEmail || "") + ">"
                                                font.pixelSize: 11
                                                color: "#7e8391"
                                            }
                                        }

                                        Text {
                                            text: backend.currentThread.date || ""
                                            font.pixelSize: 12
                                            color: "#7e8391"
                                        }
                                    }
                                }

                                // Body Text
                                Text {
                                    Layout.fillWidth: true
                                    text: backend.currentThread.body || ""
                                    font.pixelSize: 14
                                    lineHeight: 1.45
                                    color: "#d8dce6"
                                    wrapMode: Text.Wrap
                                    textFormat: Text.PlainText
                                }
                            }
                        }

                        // AI Sidebar / Drawer
                        Rectangle {
                            visible: backend.aiDrawerOpen
                            Layout.preferredWidth: 320
                            Layout.fillHeight: true
                            color: "#17181e"
                            border.color: "#24262d"
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 14

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: "✨ " + qsTr("Gist AI Assistant")
                                        font.bold: true
                                        font.pixelSize: 14
                                        color: "#ffffff"
                                    }
                                    Item { Layout.fillWidth: true }
                                    ToolButton {
                                        text: "✕"
                                        onClicked: backend.aiDrawerOpen = false
                                    }
                                }

                                // Gist Summary Box
                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 150
                                    radius: 8
                                    color: "#20222a"
                                    border.color: theme.accent
                                    border.width: 1

                                    ColumnLayout {
                                        anchors.fill: parent
                                        anchors.margins: 12
                                        spacing: 8

                                        Text {
                                            text: qsTr("Thread Summary")
                                            font.bold: true
                                            font.pixelSize: 12
                                            color: theme.accent
                                        }

                                        Text {
                                            Layout.fillWidth: true
                                            Layout.fillHeight: true
                                            text: backend.isAiBusy ? qsTr("Synthesizing gist...") : (backend.aiSummary || qsTr("Press '⚡ Get Gist' or Space to generate a summary."))
                                            font.pixelSize: 12
                                            lineHeight: 1.35
                                            color: "#ffffff"
                                            wrapMode: Text.Wrap
                                        }
                                    }
                                }

                                // Smart Reply Tone Controls
                                Text {
                                    text: qsTr("Quick Draft Replies:")
                                    font.pixelSize: 12
                                    font.bold: true
                                    color: "#9aa0af"
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Button {
                                        Layout.fillWidth: true
                                        text: qsTr("Casual")
                                        onClicked: backend.generateReply("casual")
                                    }
                                    Button {
                                        Layout.fillWidth: true
                                        text: qsTr("Brief")
                                        onClicked: backend.generateReply("brief")
                                    }
                                    Button {
                                        Layout.fillWidth: true
                                        text: qsTr("Formal")
                                        onClicked: backend.generateReply("normal")
                                    }
                                }

                                // Draft Preview
                                TextArea {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    placeholderText: qsTr("AI generated reply draft will appear here...")
                                    text: backend.aiDraft
                                    font.pixelSize: 12
                                    wrapMode: Text.Wrap
                                    color: "#ffffff"
                                    background: Rectangle {
                                        radius: 6
                                        color: "#20222a"
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------
    // Add IMAP Account Dialog
    // -------------------------------------------------------------
    Dialog {
        id: addAccountDialog
        visible: win.addAccountVisible
        anchors.centerIn: parent
        width: 440
        title: qsTr("Connect Real IMAP Account")
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAccepted: {
            if (accEmail.text.length > 0 && accHost.text.length > 0) {
                backend.addAccount(
                    "acc_" + Date.now(),
                    accName.text.length > 0 ? accName.text : accEmail.text,
                    accEmail.text,
                    accHost.text,
                    parseInt(accPort.text) || 993,
                    accUser.text.length > 0 ? accUser.text : accEmail.text,
                    accPass.text
                )
            }
            win.addAccountVisible = false
        }
        onRejected: win.addAccountVisible = false

        ColumnLayout {
            anchors.fill: parent
            spacing: 10

            TextField {
                id: accName
                Layout.fillWidth: true
                placeholderText: "Account Label (e.g. Work, Personal)"
                selectByMouse: true
            }
            TextField {
                id: accEmail
                Layout.fillWidth: true
                placeholderText: "Email Address (e.g. user@gmail.com)"
                selectByMouse: true
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                TextField {
                    id: accHost
                    Layout.fillWidth: true
                    placeholderText: "IMAP Server (e.g. imap.gmail.com)"
                    text: "imap.gmail.com"
                    selectByMouse: true
                }
                TextField {
                    id: accPort
                    Layout.preferredWidth: 80
                    placeholderText: "Port"
                    text: "993"
                    selectByMouse: true
                }
            }
            TextField {
                id: accUser
                Layout.fillWidth: true
                placeholderText: "Username (usually full email)"
                selectByMouse: true
            }
            TextField {
                id: accPass
                Layout.fillWidth: true
                placeholderText: "Password / App Password"
                echoMode: TextInput.Password
                selectByMouse: true
            }
        }
    }

    // -------------------------------------------------------------
    // Keyboard Shortcuts Dialog (?)
    // -------------------------------------------------------------
    Dialog {
        id: helpDialog
        visible: win.helpVisible
        anchors.centerIn: parent
        width: 440
        height: 380
        title: qsTr("Gist Keyboard Shortcuts")
        modal: true
        standardButtons: Dialog.Close

        onClosed: win.helpVisible = false

        ColumnLayout {
            anchors.fill: parent
            spacing: 8

            Repeater {
                model: [
                    { "key": "J / K", "desc": qsTr("Navigate threads (Down / Up)") },
                    { "key": "Space / S", "desc": qsTr("Get the Gist (AI Summary)") },
                    { "key": "R", "desc": qsTr("Generate AI Smart Reply") },
                    { "key": "E", "desc": qsTr("Archive selected thread") },
                    { "key": "#", "desc": qsTr("Delete selected thread") },
                    { "key": "C", "desc": qsTr("Compose new email") },
                    { "key": "/", "desc": qsTr("Focus search bar") },
                    { "key": "?", "desc": qsTr("Toggle this shortcut sheet") },
                    { "key": "Q", "desc": qsTr("Quit application") }
                ]

                delegate: RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Rectangle {
                        width: 70
                        height: 24
                        radius: 4
                        color: "#2a2d37"
                        Text {
                            anchors.centerIn: parent
                            text: modelData.key
                            font.bold: true
                            font.pixelSize: 11
                            color: theme.accent
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: modelData.desc
                        font.pixelSize: 12
                        color: "#ffffff"
                    }
                }
            }
        }
    }
}
