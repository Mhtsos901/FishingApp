import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Window {
    width: 500
    height: 600
    visible: true
    title: "Fishing Engine v2.0"

    Material.theme: Material.Light
    Material.accent: Material.LightBlue

    // 1. ΤΟ "ΑΥΤΙ" ΤΟΥ UI
    Connections {
        target: Backend

        // ΝΕΟ: Η παράμετρος λέγεται πλέον 'stats' και είναι JavaScript Object!
        function onCalculationFinished(surfacePct, thermoPct, thermoDepth, stats) {
            let resultString = "Επιφάνεια: <font color='#2196F3'>" + surfacePct.toFixed(1) + "%</font><br>"

            if (thermoDepth > 0) {
                // Αν βρήκαμε θερμοκλίνα, δείχνουμε το ιδανικό βάθος
                resultString += "Στα " + thermoDepth.toFixed(0) + " μέτρα: <font color='#4CAF50'><b>" + thermoPct.toFixed(1) + "%</b></font>"
            } else {
                resultString += "<font color='#757575'>Η λίμνη είναι πλήρως ανακατεμένη.</font>"
            }

            // --- ΕΔΩ ΤΟ QML ΧΤΙΖΕΙ ΤΟ ΓΡΑΦΙΚΟ ΚΟΜΜΑΤΙ (View Logic) ---
            let detailsHtml = "";

            // Έλεγχος για το αν υπάρχει θερμοκλίνα
            if (stats.thermoclineDepth > 0.0) {
                detailsHtml += `Βάθος Επιλιμνίου: 0 έως ${stats.thermoclineDepth.toFixed(1)} m<br>`;
                detailsHtml += `Θερμ. Νερού (Επιφάνεια): ${stats.surfaceTemp.toFixed(1)} °C<br>`;
                detailsHtml += `Θερμ. Νερού (${stats.bestDepth}m): ${stats.bestTemp.toFixed(1)} °C<br>`;
            } else {
                detailsHtml += `Βάθος Επιλιμνίου: Η λίμνη είναι ανακατεμένη<br>`;
                detailsHtml += `Θερμ. Νερού (Επιφάνεια): ${stats.surfaceTemp.toFixed(1)} °C<br>`;
            }

            // Προσθήκη των κοινών στατιστικών (αέρα, βροχής, πίεσης)
            detailsHtml += `Θερμ. Αέρα: ${stats.airTemp.toFixed(1)} °C<br>`;
            detailsHtml += `Αέρας: ${stats.beaufort} Μποφόρ - ${stats.windKmh.toFixed(1)} km/h [${stats.compassDir}]<br>`;
            detailsHtml += `Βροχή: ${stats.rain.toFixed(1)} mm<br>`;
            detailsHtml += `Βαρόμετρο: ${stats.pressure.toFixed(1)} hPa`;

            // Ενημέρωση του UI
            resultText.textFormat = Text.RichText
            resultText.text = resultString + "<br><br><font size='3' color='#555555'>" + detailsHtml + "</font>"
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

        // --- ΠΛΑΙΣΙΟ ΑΠΟΤΕΛΕΣΜΑΤΟΣ ΜΕ ΒΕΛΟΣ ---
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