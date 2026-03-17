import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material

Window {
    width: 450
    height: 750
    visible: true
    title: "Fishing Engine v2.0"

    // Απαλό γκρι background για να ξεχωρίζει η λευκή κάρτα
    color: "#F4F6F8"

    Material.theme: Material.Light
    Material.accent: Material.LightBlue
    Material.primary: Material.Blue

    // --- ΝΕΟ: Ιδιότητα για το χρώμα του βέλους (Default: Μπλε) ---
    property string arrowColor: "#2196F3"

    // 1. ΤΟ "ΑΥΤΙ" ΤΟΥ UI
    Connections {
        target: Backend
        function onCalculationFinished(surfacePct, thermoPct, thermoDepth, stats) {

            // --- ΝΕΟ: Helper συνάρτηση για τα Χρώματα (Visual Telemetry) ---
            function getScoreColor(score) {
                if (score >= 0.8) return "#4CAF50"; // Πράσινο (Τέλειο)
                if (score >= 0.6) return "#8BC34A"; // Ανοιχτό Πράσινο (Καλό)
                if (score >= 0.4) return "#FFC107"; // Κίτρινο (Μέτριο)
                if (score >= 0.2) return "#FF9800"; // Πορτοκαλί (Κακό)
                return "#F44336"; // Κόκκινο (Τραγικό)
            }

            // Χρωματίζουμε δυναμικά το βέλος του ανέμου!
            arrowColor = getScoreColor(stats.scoreWindDir);

            // Παίρνουμε τα χρώματα για τα κείμενα
            let tempColor = getScoreColor(stats.scoreTemp);
            let rainColor = getScoreColor(stats.scoreRain);
            let pressColor = getScoreColor(stats.scorePressure);

            let resultString = "Επιφάνεια: <font color='#2196F3'>" + surfacePct.toFixed(1) + "%</font><br>"

            if (thermoDepth > 0) {
                resultString += "Στα " + thermoDepth.toFixed(0) + " μέτρα: <font color='#4CAF50'><b>" + thermoPct.toFixed(1) + "%</b></font>"
            } else {
                resultString += "<font color='#757575'>Η λίμνη είναι πλήρως ανακατεμένη.</font>"
            }

            let detailsHtml = "";

            // --- ΝΕΟ: Εφαρμόζουμε τα χρώματα (<font color=...>) ---
            if (stats.thermoclineDepth > 0.0) {
                detailsHtml += `Βάθος Επιλιμνίου: 0 έως ${stats.thermoclineDepth.toFixed(1)} m<br>`;
                detailsHtml += `Θερμ. Νερού (Επιφάνεια): <font color='${tempColor}'><b>${stats.surfaceTemp.toFixed(1)} °C</b></font><br>`;
                detailsHtml += `Θερμ. Νερού (${stats.bestDepth}m): <font color='${tempColor}'><b>${stats.bestTemp.toFixed(1)} °C</b></font><br>`;
            } else {
                detailsHtml += `Βάθος Επιλιμνίου: Η λίμνη είναι ανακατεμένη<br>`;
                detailsHtml += `Θερμ. Νερού (Επιφάνεια): <font color='${tempColor}'><b>${stats.surfaceTemp.toFixed(1)} °C</b></font><br>`;
            }

            detailsHtml += `Θερμ. Αέρα: ${stats.airTemp.toFixed(1)} °C<br>`;
            detailsHtml += `Αέρας: ${stats.beaufort} Μποφόρ - ${stats.windKmh.toFixed(1)} km/h [${stats.compassDir}]<br>`;
            detailsHtml += `Βροχή: <font color='${rainColor}'><b>${stats.rain.toFixed(1)} mm</b></font><br>`;
            detailsHtml += `Βαρόμετρο: <font color='${pressColor}'><b>${stats.pressure.toFixed(1)} hPa</b></font>`;

            resultText.textFormat = Text.RichText
            resultText.text = resultString + "<br><br><font size='3' color='#555555'>" + detailsHtml + "</font>"
        }

        function onCalculationError(errorMessage) {
            resultText.text = "<font color='#D32F2F'><b>Σφάλμα:</b><br>" + errorMessage + "</font>"
        }
    }

    // 2. ΣΧΕΔΙΑΣΜΟΣ ΟΘΟΝΗΣ
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 30
        spacing: 20

        // --- Header / Τίτλος ---
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 5

            Text {
                text: "🐟 Fishing Engine"
                font.pixelSize: 28
                font.bold: true
                color: "#1976D2"
                Layout.alignment: Qt.AlignHCenter
            }
            Text {
                text: "v2.0 Simulation Model"
                font.pixelSize: 14
                color: "#757575"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // --- Κάρτα Ρυθμίσεων (Material Card) ---
        Pane {
            Layout.fillWidth: true
            Material.elevation: 4
            padding: 25

            ColumnLayout {
                width: parent.width
                spacing: 15

                Text {
                    text: "ΤΟΠΟΘΕΣΙΑ"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#757575"
                }
                ComboBox {
                    id: lakeCombo
                    Layout.fillWidth: true
                    font.pixelSize: 16
                    textRole: "text"
                    valueRole: "value"
                    model: ListModel {
                        ListElement { text: "Τριχωνίδα"; value: "trichonida" }
                        ListElement { text: "Ρίβιο"; value: "rivio" }
                        ListElement { text: "Οζερός"; value: "ozeros" }
                    }
                }

                Text {
                    text: "ΕΙΔΟΣ ΨΑΡΙΟΥ"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#757575"
                    Layout.topMargin: 10
                }
                ComboBox {
                    id: fishCombo
                    Layout.fillWidth: true
                    font.pixelSize: 16
                    textRole: "text"
                    valueRole: "value"
                    model: ListModel {
                        ListElement { text: "Γριβάδι"; value: "carp" }
                        ListElement { text: "Πεταλούδα"; value: "petalouda" }
                    }
                }

                Button {
                    text: "ΥΠΟΛΟΓΙΣΜΟΣ ΠΙΘΑΝΟΤΗΤΑΣ"
                    Layout.fillWidth: true
                    Layout.topMargin: 20
                    Layout.preferredHeight: 45
                    font.pixelSize: 15
                    font.bold: true
                    Material.background: Material.accent
                    Material.foreground: "white"
                    Material.elevation: 2

                    onClicked: {
                        resultText.text = "Γίνεται λήψη καιρού..."
                        Backend.calculateCatchProbability(lakeCombo.currentValue, fishCombo.currentValue)
                    }
                }
            }
        }

        // --- ΠΛΑΙΣΙΟ ΑΠΟΤΕΛΕΣΜΑΤΟΣ ΜΕ ΒΕΛΟΣ ---
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 15
            Layout.fillHeight: true
            spacing: 20
            visible: resultText.text !== "Αναμονή επιλογής..." && resultText.text !== "Γίνεται λήψη καιρού..."

            Text {
                text: "↑"
                font.pixelSize: 54
                font.bold: true
                // --- ΝΕΟ: Το χρώμα τραβάει τη μεταβλητή arrowColor ---
                color: arrowColor
                rotation: Backend.windDegrees + 180
                Behavior on rotation {
                    NumberAnimation { duration: 800; easing.type: Easing.OutBack }
                }
            }

            Text {
                id: resultText
                text: "Αναμονή επιλογής..."
                font.pixelSize: 15
                lineHeight: 1.3
                horizontalAlignment: Text.AlignLeft
                Layout.fillWidth: true
            }
        }
    }
}