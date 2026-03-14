import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Window {
    width: 400
    height: 600
    visible: true
    title: "Fishing Engine v2.0"

    Material.theme: Material.Light
    Material.accent: Material.LightBlue

    // 1. ΤΟ "ΑΥΤΙ" ΤΟΥ UI
    Connections {
        target: Backend
        function onCalculationFinished(percentage, debugInfo) {
            resultText.text = "Πιθανότητα: " + percentage.toFixed(1) + "%\n\n" + debugInfo
        }
        function onCalculationError(errorMessage) {
            resultText.text = "Σφάλμα:\n" + errorMessage
        }
    }

    // 2. ΣΧΕΔΙΑΣΜΟΣ ΟΘΟΝΗΣ
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 20
        width: 320

        Text {
            text: "Ρυθμίσεις Ψαρέματος"
            font.pixelSize: 22
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 10
        }

        ComboBox {
            id: lakeCombo
            Layout.fillWidth: true
            font.pixelSize: 16
            textRole: "text"
            valueRole: "value"
            model: ListModel {
                ListElement { text: "Τριχωνίδα"; value: 1 }
                ListElement { text: "Ρίβιο"; value: 2 }
                ListElement { text: "Οζερός"; value: 3 }
            }
        }

        ComboBox {
            id: fishCombo
            Layout.fillWidth: true
            font.pixelSize: 16
            textRole: "text"
            valueRole: "value"
            model: ListModel {
                ListElement { text: "Γριβάδι"; value: 1 }
                ListElement { text: "Πεταλούδα"; value: 2 }
            }
        }

        Button {
            text: "Υπολογισμός Πιθανότητας"
            Layout.fillWidth: true
            Layout.topMargin: 15
            font.pixelSize: 18
            Material.background: Material.accent
            Material.foreground: "white"

            onClicked: {
                resultText.text = "Γίνεται λήψη καιρού..."
                Backend.calculateCatchProbability(lakeCombo.currentValue, fishCombo.currentValue)
            }
        }

        // --- ΝΕΟ: ΠΛΑΙΣΙΟ ΑΠΟΤΕΛΕΣΜΑΤΟΣ ΜΕ ΒΕΛΟΣ ---
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 25
            spacing: 20
            // Το βέλος και το κείμενο εμφανίζονται μόνο όταν έχουμε αποτέλεσμα
            visible: resultText.text !== "Αναμονή επιλογής..." && resultText.text !== "Γίνεται λήψη καιρού..."

            // Το Βέλος Κατεύθυνσης Αέρα
            Text {
                text: "↑"
                font.pixelSize: 54
                font.bold: true
                color: Material.accent

                // Σύνδεση με τη C++ Property: Backend.windDegrees
                // Προσθέτουμε 180 για να δείχνει ΠΟΥ πάει ο αέρας
                rotation: Backend.windDegrees + 180

                // Ομαλή κίνηση περιστροφής (Animation)
                Behavior on rotation {
                    NumberAnimation { duration: 800; easing.type: Easing.OutBack }
                }
            }

            // Το κείμενο με τις λεπτομέρειες
            Text {
                id: resultText
                text: "Αναμονή επιλογής..."
                font.pixelSize: 15
                lineHeight: 1.2
                horizontalAlignment: Text.AlignLeft
                Layout.fillWidth: true
            }
        }
    }
}