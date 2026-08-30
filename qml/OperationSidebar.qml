import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import ModernHAMLoggerQt

Rectangle {
    id: root
    color: Theme.panelBackground
    Layout.fillHeight: true
    Layout.preferredWidth: 220
    Layout.minimumWidth: 160

    function requestDelete(index, name) {
        confirmDelete.pendingIndex = index
        confirmDelete.pendingName = name
        confirmDelete.open()
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        Label {
            text: qsTr("Operations")
            color: Theme.textOnPanel
            font.bold: true
            font.pixelSize: 16
            Layout.fillWidth: true
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: LogbookManager.operations

            delegate: ItemDelegate {
                width: listView.width
                highlighted: index === LogbookManager.currentOperationIndex

                contentItem: RowLayout {
                    spacing: 6

                    ColumnLayout {
                        spacing: 2
                        Layout.fillWidth: true

                        Label {
                            text: model.name
                            color: Theme.textOnPanel
                            font.bold: index === LogbookManager.currentOperationIndex
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        Label {
                            text: {
                                var parts = [qsTr("%n QSO(s)", "", model.qsoCount)]
                                if (model.potaRef)
                                    parts.push("POTA " + model.potaRef)
                                if (model.sotaRef)
                                    parts.push("SOTA " + model.sotaRef)
                                return parts.join(" · ")
                            }
                            color: Theme.textOnPanel
                            opacity: 0.7
                            font.pixelSize: 11
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }

                    Text {
                        text: "✕"
                        font.pixelSize: 13
                        font.bold: true
                        color: deleteHover.hovered ? "#ff6b6b" : "#c04040"

                        HoverHandler {
                            id: deleteHover
                            cursorShape: Qt.PointingHandCursor
                        }
                        TapHandler {
                            onTapped: root.requestDelete(index, model.name)
                        }
                    }
                }

                onClicked: LogbookManager.selectOperation(index)
            }

            ScrollBar.vertical: ScrollBar {}
        }

        Button {
            text: qsTr("+ New Operation")
            Layout.fillWidth: true
            onClicked: newOperationDialog.openForCreate()
        }
    }

    NewOperationDialog {
        id: newOperationDialog
    }

    Dialog {
        id: confirmDelete
        property int pendingIndex: -1
        property string pendingName: ""
        title: qsTr("Delete Operation?")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent

        contentItem: Label {
            text: qsTr("Permanently delete \"%1\" and all its logged QSOs? This cannot be undone.").arg(confirmDelete.pendingName)
            wrapMode: Text.Wrap
        }

        onAccepted: {
            LogbookManager.deleteOperation(pendingIndex)
            pendingIndex = -1
            pendingName = ""
        }
        onRejected: {
            pendingIndex = -1
            pendingName = ""
        }
    }
}
