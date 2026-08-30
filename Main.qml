import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import ModernHAMLoggerQt

ApplicationWindow {
    id: window
    width: 1024
    height: 680
    minimumWidth: 640
    minimumHeight: 420
    visible: true
    title: qsTr("ModernHAMLoggerQt")

    // Controls.Basic draws itself from the palette, so theming the window
    // palette is what makes buttons/fields follow the dark/light toggle.
    palette.window: Theme.windowBackground
    palette.windowText: Theme.textColor
    palette.base: Theme.fieldBackground
    palette.alternateBase: Theme.alternateRow
    palette.text: Theme.textColor
    palette.button: Theme.buttonBackground
    palette.buttonText: Theme.textColor
    palette.mid: Theme.borderColor
    palette.dark: Theme.borderColor
    palette.light: Theme.buttonBackground
    palette.midlight: Theme.headerBackground
    palette.highlight: Theme.accent
    palette.highlightedText: "#ffffff"
    palette.placeholderText: Theme.placeholder
    palette.toolTipBase: Theme.panelBackground
    palette.toolTipText: Theme.textColor

    header: ToolBar {
        // padding (not anchors.margins on the RowLayout below) is what the
        // ToolBar/Pane implicitHeight calculation actually accounts for --
        // anchors.margins on a fill-anchored child is invisible to it, so the
        // toolbar was reserving only the buttons' own height and the margin
        // pushed them past its bounds into the form underneath.
        padding: 6

        background: Rectangle {
            color: Theme.panelBackground
            implicitHeight: 40
        }

        RowLayout {
            anchors.fill: parent
            spacing: 8

            Label {
                text: qsTr("ModernHAMLoggerQt")
                color: Theme.textOnPanel
                font.bold: true
                font.pixelSize: 16
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Station Profile")
                onClicked: stationProfileDialog.openForEdit()
            }

            Button {
                text: qsTr("Import ADIF")
                onClicked: importDialog.open()
            }

            Button {
                text: qsTr("Export ADIF")
                enabled: LogbookManager.currentOperationIndex >= 0
                onClicked: exportDialog.open()
            }

            Button {
                text: Theme.lightMode ? qsTr("☽  Dark mode") : qsTr("☼  Light mode")
                onClicked: Theme.lightMode = !Theme.lightMode
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        OperationSidebar {}

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            EntryForm {}
            LogTable {}
        }
    }

    StationProfileDialog {
        id: stationProfileDialog
    }

    FileDialog {
        id: importDialog
        title: qsTr("Import ADIF Log")
        nameFilters: [qsTr("ADIF files (*.adi *.adif)"), qsTr("All files (*)")]
        onAccepted: LogbookManager.importAdif(selectedFile)
    }

    FileDialog {
        id: exportDialog
        title: qsTr("Export ADIF Log")
        fileMode: FileDialog.SaveFile
        nameFilters: [qsTr("ADIF files (*.adi)")]
        defaultSuffix: "adi"
        onAccepted: LogbookManager.exportAdif(selectedFile, LogbookManager.currentOperationIndex)
    }
}
