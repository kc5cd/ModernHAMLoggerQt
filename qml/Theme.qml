pragma Singleton
import QtQuick

QtObject {
    property bool lightMode: Application.styleHints.colorScheme === Qt.Light

    readonly property color reallyDark: "#1f1f1f"
    readonly property color dark: "#262626"
    readonly property color reallyLight: "#e7e7e7"
    readonly property color light: "#e0e0e0"

    readonly property color windowBackground: lightMode ? reallyLight : reallyDark
    readonly property color panelBackground: lightMode ? light : dark

    // Foreground must contrast against the backgrounds above, so both of these
    // resolve to the opposite end of the palette from windowBackground.
    readonly property color textColor: lightMode ? "#1a1a1a" : "#e7e7e7"
    readonly property color textOnWindow: textColor
    readonly property color textOnPanel: textColor

    readonly property color fieldBackground: lightMode ? "#ffffff" : "#303030"
    readonly property color buttonBackground: lightMode ? "#d4d4d4" : "#3a3a3a"
    readonly property color borderColor: lightMode ? "#b0b0b0" : "#4a4a4a"
    readonly property color headerBackground: lightMode ? "#cdcdcd" : "#333333"
    readonly property color alternateRow: lightMode ? "#d8d8d8" : "#2c2c2c"
    readonly property color accent: lightMode ? "#2c6bbf" : "#3a7bd5"
    readonly property color placeholder: lightMode ? "#8a8a8a" : "#8f8f8f"
}
