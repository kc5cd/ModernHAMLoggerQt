import QtQuick
import QtTest

TestCase {
    name: "Smoke"

    function test_harness_runs() {
        compare(2 + 2, 4)
    }
}
