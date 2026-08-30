import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import QtQml.Models
import ModernHAMLoggerQt

Rectangle {
    id: root
    color: Theme.panelBackground
    Layout.fillWidth: true
    Layout.fillHeight: true

    readonly property var currentLog: LogbookManager.currentOperation ? LogbookManager.currentOperation.log : null

    // Indexed by QsoLogModel::Column, TimeColumn..CountryColumn. NotesColumn
    // (the last column) is not listed here -- it stretches to fill whatever
    // width is left over, see tableView.columnWidthProvider below.
    readonly property var fixedColumnWidths: [140, 100, 70, 75, 95, 60, 60, 150, 80, 130]
    readonly property int notesMinWidth: 240
    readonly property int fixedTotal: fixedColumnWidths.reduce((a, b) => a + b, 0)

    readonly property int gutterWidth: 56

    // Row indices checked via the gutter checkboxes. A plain array, not a JS
    // Set: reassigning it is what re-evaluates delegate bindings, while
    // mutating a Set in place emits no QML change notification.
    property var checkedRows: []
    readonly property bool allChecked: tableView.rows > 0 && checkedRows.length === tableView.rows

    // Backs the shared Notes tooltip below; set by the cell delegate's hover handler.
    property string hoveredNote: ""

    onCurrentLogChanged: checkedRows = []

    Connections {
        target: root.currentLog
        ignoreUnknownSignals: true
        function onRowsRemoved() { root.checkedRows = [] }
        function onModelReset() { root.checkedRows = [] }
        // rowsInserted intentionally unhandled: addQso only ever appends, so
        // existing checked indices stay valid when a new contact is logged.
    }

    function isChecked(row) {
        return checkedRows.indexOf(row) !== -1
    }

    function setChecked(row, on) {
        checkedRows = on ? checkedRows.concat(row) : checkedRows.filter(r => r !== row)
    }

    function checkAll() {
        let all = []
        for (let i = 0; i < tableView.rows; ++i)
            all.push(i)
        checkedRows = all
    }

    function uncheckAll() {
        checkedRows = []
    }

    // Single entry point for both the per-row X and the bulk delete button,
    // so deleteQsos() has exactly one call site and always goes through the
    // confirmation dialog -- there is no undo for a deleted QSO.
    function requestDelete(rows) {
        if (rows.length === 0)
            return
        confirmDelete.pendingRows = rows
        confirmDelete.open()
    }

    Dialog {
        id: confirmDelete
        property var pendingRows: []
        title: qsTr("Delete QSOs?")
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        anchors.centerIn: parent

        contentItem: Label {
            text: qsTr("Permanently delete %n QSO(s)? This cannot be undone.",
                        "", confirmDelete.pendingRows.length)
            wrapMode: Text.Wrap
        }

        onAccepted: {
            LogbookManager.deleteQsos(LogbookManager.currentOperationIndex, pendingRows)
            pendingRows = []
            root.checkedRows = []
        }
        onRejected: pendingRows = []
    }

    // One shared tooltip for the whole grid rather than a per-cell ToolTip
    // attached property: cheaper (no popup per recycled delegate) and gives
    // control over wrapping for a long note.
    ToolTip {
        visible: root.hoveredNote.length > 0
        delay: 500

        contentItem: Text {
            text: root.hoveredNote
            color: Theme.textColor
            wrapMode: Text.Wrap
            width: Math.min(implicitWidth, 420)
        }
        background: Rectangle {
            color: Theme.panelBackground
            border.color: Theme.borderColor
            border.width: 1
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4

        Label {
            text: qsTr("Double-click a cell to edit it.")
            color: Theme.textColor
            opacity: 0.7
            font.pixelSize: 11
            visible: tableView.rows > 0
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Delete/select gutter, pinned left of the horizontally-scrolling
            // table via VerticalHeaderView + syncView so it never scrolls out
            // of view.
            ColumnLayout {
                Layout.fillHeight: true
                Layout.preferredWidth: root.gutterWidth
                spacing: 0

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: headerView.height
                    color: Theme.headerBackground
                    border.width: 1
                    border.color: Theme.borderColor

                    CheckIndicator {
                        anchors.centerIn: parent
                        checked: root.allChecked
                        onToggled: root.allChecked ? root.uncheckAll() : root.checkAll()
                    }
                }

                VerticalHeaderView {
                    id: gutter
                    syncView: tableView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    resizableRows: false
                    resizableColumns: false // defaults true on VerticalHeaderView
                    pointerNavigationEnabled: false
                    keyNavigationEnabled: false
                    columnWidthProvider: () => root.gutterWidth

                    delegate: Rectangle {
                        id: gutterCell
                        required property int row
                        implicitHeight: 26
                        implicitWidth: root.gutterWidth
                        color: row % 2 === 0 ? Theme.panelBackground : Theme.alternateRow

                        Row {
                            anchors.centerIn: parent
                            spacing: 6

                            Text {
                                text: "✕"
                                font.pixelSize: 13
                                font.bold: true
                                color: xHover.hovered ? "#ff6b6b" : "#c04040"

                                HoverHandler {
                                    id: xHover
                                    cursorShape: Qt.PointingHandCursor
                                }
                                TapHandler {
                                    onTapped: root.requestDelete([gutterCell.row])
                                }
                            }

                            CheckIndicator {
                                checked: root.isChecked(gutterCell.row)
                                onToggled: root.setChecked(gutterCell.row, !root.isChecked(gutterCell.row))
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 4

                HorizontalHeaderView {
                    id: headerView
                    syncView: tableView
                    Layout.fillWidth: true
                    clip: true

                    delegate: Rectangle {
                        required property string display
                        implicitHeight: 26
                        color: Theme.headerBackground
                        border.width: 1
                        border.color: Theme.borderColor

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 4
                            text: parent.display
                            color: Theme.textColor
                            font.bold: true
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }

                TableView {
                    id: tableView
                    Layout.fillWidth: true
                    Layout.preferredWidth: 0 // avoid a width<->contentWidth feedback loop, see columnWidthProvider
                    Layout.fillHeight: true
                    clip: true
                    model: root.currentLog
                    columnSpacing: 1
                    rowSpacing: 1
                    editTriggers: TableView.DoubleTapped | TableView.EditKeyPressed

                    // All columns but Notes are fixed width; Notes absorbs
                    // whatever width is left over so the grid always fills
                    // the available horizontal space.
                    columnWidthProvider: function (column) {
                        if (column !== QsoLogModel.NotesColumn)
                            return root.fixedColumnWidths[column]
                        const gaps = tableView.columnSpacing * (tableView.columns - 1)
                        return Math.max(root.notesMinWidth,
                                        Math.floor(tableView.width - root.fixedTotal - gaps))
                    }
                    // columnWidthProvider is a plain function -- reading
                    // tableView.width inside it creates no binding, so a
                    // resize needs an explicit forceLayout(). Qt.callLater
                    // coalesces a drag-resize's many widthChanged firings
                    // into one relayout per event-loop pass.
                    onWidthChanged: Qt.callLater(forceLayout)

                    selectionBehavior: TableView.SelectRows
                    selectionMode: TableView.SingleSelection
                    selectionModel: ItemSelectionModel { model: tableView.model }

                    delegate: Rectangle {
                        id: cell
                        required property string display
                        required property bool selected
                        required property int row
                        required property int column

                        implicitHeight: 26
                        color: selected ? Theme.accent
                                        : (row % 2 === 0 ? Theme.panelBackground : Theme.alternateRow)

                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 4
                            anchors.rightMargin: 4
                            text: cell.display
                            color: cell.selected ? "#ffffff" : Theme.textColor
                            elide: Text.ElideRight
                            verticalAlignment: Text.AlignVCenter
                        }

                        HoverHandler {
                            enabled: cell.column === QsoLogModel.NotesColumn && cell.display.length > 0
                            onHoveredChanged: {
                                if (hovered)
                                    root.hoveredNote = cell.display
                                else if (root.hoveredNote === cell.display)
                                    root.hoveredNote = ""
                            }
                        }

                        TableView.editDelegate: TextField {
                            anchors.fill: parent
                            text: cell.display
                            Component.onCompleted: selectAll()
                            TableView.onCommit: root.currentLog.setCell(cell.row, cell.column, text)
                        }
                    }

                    ScrollBar.vertical: ScrollBar {}
                    ScrollBar.horizontal: ScrollBar {}
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: qsTr("%n QSO(s)", "", tableView.rows)
                color: Theme.textColor
                opacity: 0.7
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Delete Checked (%1)").arg(root.checkedRows.length)
                enabled: root.checkedRows.length > 0
                onClicked: root.requestDelete(root.checkedRows)
            }
        }
    }
}
