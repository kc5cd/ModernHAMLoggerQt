import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import ModernHAMLoggerQt

Rectangle {
    id: root
    color: Theme.windowBackground
    Layout.fillWidth: true
    // Derived from the margin actually applied below, rather than a
    // hand-tuned constant that would silently drift if the margin changes.
    implicitHeight: formColumn.implicitHeight + formColumn.anchors.margins * 2

    readonly property bool hasOperation: LogbookManager.currentOperationIndex >= 0
    readonly property var bands: ["160m", "80m", "60m", "40m", "30m", "20m", "17m", "15m", "12m", "10m", "6m", "2m", "70cm"]
    readonly property var modes: ["SSB", "FM", "AM", "CW", "FT8", "FT4", "RTTY", "PSK31", "DMR"]

    function defaultRst(mode) {
        return (mode === "SSB" || mode === "FM" || mode === "AM") ? "59" : "599"
    }

    function logContact() {
        if (!hasOperation || callsignField.text.trim().length === 0)
            return

        LogbookManager.logQso({
            callsign: callsignField.text.trim().toUpperCase(),
            band: bandCombo.currentText,
            mode: modeCombo.currentText,
            freqMhz: (parseFloat(freqField.text) || 0) / 1000,
            rstSent: rstSentField.text,
            rstRcvd: rstRcvdField.text,
            name: nameField.text.trim(),
            gridSquare: gridField.text.trim().toUpperCase(),
            country: countryField.text.trim(),
            notes: notesField.text.trim()
        })

        callsignField.text = ""
        nameField.text = ""
        gridField.text = ""
        countryField.text = ""
        notesField.text = ""
        callsignField.forceActiveFocus()
    }

    Connections {
        target: CallsignLookup
        function onLookupSucceeded(callsign, data) {
            if (callsign !== callsignField.text.trim().toUpperCase())
                return
            if (data.name)
                nameField.text = data.name
            if (data.gridSquare)
                gridField.text = data.gridSquare
            if (data.country)
                countryField.text = data.country
        }
    }

    ColumnLayout {
        id: formColumn
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        Label {
            visible: !root.hasOperation
            text: qsTr("Select or create an operation to start logging contacts.")
            color: Theme.textOnWindow
            Layout.fillWidth: true
        }

        FontMetrics {
            id: fieldMetrics
        }

        RowLayout {
            spacing: 8
            Layout.fillWidth: true
            enabled: root.hasOperation

            Label { text: qsTr("Callsign"); color: Theme.textOnWindow }
            TextField {
                id: callsignField
                Layout.preferredWidth: fieldMetrics.averageCharacterWidth * 12 + 16
                maximumLength: 12
                onTextChanged: {
                    if (text !== text.toUpperCase()) {
                        const pos = cursorPosition
                        text = text.toUpperCase()
                        cursorPosition = pos
                    }
                }
                onEditingFinished: if (text.trim().length > 0) CallsignLookup.lookup(text.trim())
                Keys.onReturnPressed: root.logContact()
            }

            Label { text: qsTr("RST Sent"); color: Theme.textOnWindow }
            TextField {
                id: rstSentField
                Layout.preferredWidth: fieldMetrics.averageCharacterWidth * 4 + 16
                maximumLength: 4
                text: "59"
            }

            Label { text: qsTr("RST Rcvd"); color: Theme.textOnWindow }
            TextField {
                id: rstRcvdField
                Layout.preferredWidth: fieldMetrics.averageCharacterWidth * 4 + 16
                maximumLength: 4
                text: "59"
            }

            Label { text: qsTr("Freq (kHz)"); color: Theme.textOnWindow }
            TextField {
                id: freqField
                Layout.preferredWidth: fieldMetrics.averageCharacterWidth * 12 + 16
                maximumLength: 12
                validator: IntValidator { bottom: 0; top: 99999999 }
            }

            Label { text: qsTr("Mode"); color: Theme.textOnWindow }
            ComboBox {
                id: modeCombo
                Layout.preferredWidth: fieldMetrics.averageCharacterWidth * 8 + 40
                model: root.modes
                onCurrentTextChanged: {
                    rstSentField.text = root.defaultRst(currentText)
                    rstRcvdField.text = root.defaultRst(currentText)
                }
            }

            Label { text: qsTr("Band"); color: Theme.textOnWindow }
            ComboBox {
                id: bandCombo
                Layout.preferredWidth: fieldMetrics.averageCharacterWidth * 8 + 40
                model: root.bands
                currentIndex: 4
            }

            Item { Layout.fillWidth: true }
        }

        GridLayout {
            columns: 6
            columnSpacing: 8
            rowSpacing: 4
            Layout.fillWidth: true
            enabled: root.hasOperation

            Label { text: qsTr("Name"); color: Theme.textOnWindow }
            TextField { id: nameField; Layout.fillWidth: true }

            Label { text: qsTr("Grid"); color: Theme.textOnWindow }
            TextField {
                id: gridField
                // averageCharacterWidth undersizes all-caps content like grid
                // squares ("XOXOXO") -- measure the actual widest glyph
                // repeated to the field's max length instead.
                Layout.preferredWidth: fieldMetrics.horizontalAdvance("M".repeat(maximumLength)) + 16
                maximumLength: 8
            }

            Label { text: qsTr("Country"); color: Theme.textOnWindow }
            TextField { id: countryField; Layout.fillWidth: true; readOnly: true }

            Label { text: qsTr("Notes"); color: Theme.textOnWindow }
            TextField {
                id: notesField
                Layout.columnSpan: 5
                Layout.fillWidth: true
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: qsTr("Log Contact")
                enabled: root.hasOperation && callsignField.text.trim().length > 0
                onClicked: root.logContact()
            }
        }
    }
}
