import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import ModernHAMLoggerQt

Dialog {
    id: root
    title: qsTr("New Operation")
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

    function openForCreate() {
        nameField.text = ""
        potaField.text = ""
        sotaField.text = ""
        open()
        nameField.forceActiveFocus()
    }

    onAccepted: {
        if (nameField.text.trim().length === 0)
            return
        LogbookManager.addOperation(nameField.text.trim(), potaField.text.trim(), sotaField.text.trim())
    }

    contentItem: ColumnLayout {
        spacing: 8

        Label { text: qsTr("Name") }
        TextField {
            id: nameField
            Layout.fillWidth: true
            placeholderText: qsTr("e.g. Home Station, POTA K-1234")
        }

        Label { text: qsTr("POTA reference (optional)") }
        TextField {
            id: potaField
            Layout.fillWidth: true
            placeholderText: qsTr("e.g. K-1234")
        }

        Label { text: qsTr("SOTA reference (optional)") }
        TextField {
            id: sotaField
            Layout.fillWidth: true
            placeholderText: qsTr("e.g. W1/AB-001")
        }
    }
}
