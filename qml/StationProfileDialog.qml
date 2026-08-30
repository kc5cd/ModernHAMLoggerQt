import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import ModernHAMLoggerQt

Dialog {
    id: root
    title: qsTr("Station Profile")
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

    function openForEdit() {
        callsignField.text = LogbookManager.profile.callsign
        nameField.text = LogbookManager.profile.operatorName
        gridField.text = LogbookManager.profile.gridSquare
        open()
        callsignField.forceActiveFocus()
    }

    onAccepted: {
        LogbookManager.profile.callsign = callsignField.text.trim().toUpperCase()
        LogbookManager.profile.operatorName = nameField.text.trim()
        LogbookManager.profile.gridSquare = gridField.text.trim().toUpperCase()
        LogbookManager.profile.save()
    }

    contentItem: ColumnLayout {
        spacing: 8

        Label { text: qsTr("Your Callsign") }
        TextField {
            id: callsignField
            Layout.fillWidth: true
        }

        Label { text: qsTr("Operator Name") }
        TextField {
            id: nameField
            Layout.fillWidth: true
        }

        Label { text: qsTr("Grid Square") }
        TextField {
            id: gridField
            Layout.fillWidth: true
            placeholderText: qsTr("e.g. FN31pr")
        }
    }
}
