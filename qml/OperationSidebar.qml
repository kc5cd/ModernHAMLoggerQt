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

                contentItem: ColumnLayout {
                    spacing: 2
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
}
