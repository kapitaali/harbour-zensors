import QtQuick 2.6
import Sailfish.Silica 1.0
import QtTest 1.2
import "../../qml/pages"

/*
 * Qt Quick Test skeleton.
 * Run inside the build engine / emulator environment with qmltestrunner,
 * or let CI pick it up via tests.xml (testrunnerlite).
 *
 * Conventions (docs.sailfishos.org/Develop/Apps/Coding_Conventions):
 *  - prefer compare() over verify()
 *  - use SignalSpy instead of wait()
 */
TestCase {
    id: testCase
    name: "FirstPageTests"
    when: windowShown

    function initTestCase() {
        // setup once
    }

    function test_page_loads() {
        var page = createTemporaryObject(FirstPage, testCase)
        compare(page.allowedOrientations, Orientation.All)
    }

    function test_header_title() {
        var page = createTemporaryObject(FirstPage, testCase)
        compare(page.children[0].children[1].text, "My App")
    }

    SignalSpy {
        id: spy
        signalName: "onPageContainerChanged"
    }
}
