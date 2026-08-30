import QtQuick
import ModernHAMLoggerQt

// A stateless checkbox-like indicator safe for use inside recycled TableView /
// VerticalHeaderView delegates.
//
// Controls.CheckBox writes its own `checked` property when clicked, which
// permanently breaks an inbound binding such as `checked: root.isChecked(row)`
// -- and because TableView/VerticalHeaderView recycle delegates, a broken
// binding shows the wrong row's check state after scrolling. This component
// keeps `checked` a pure one-way binding: clicking it never assigns `checked`
// itself, it only emits `toggled()` for the caller to act on.
Rectangle {
    id: root

    property bool checked: false
    signal toggled()

    implicitWidth: 16
    implicitHeight: 16
    radius: 3
    color: checked ? Theme.accent : Theme.fieldBackground
    border.width: 1
    border.color: hoverHandler.hovered ? Theme.accent : Theme.borderColor

    Text {
        anchors.centerIn: parent
        text: "✓"
        color: "#ffffff"
        font.pixelSize: 11
        font.bold: true
        visible: root.checked
    }

    HoverHandler {
        id: hoverHandler
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        onTapped: root.toggled()
    }
}
