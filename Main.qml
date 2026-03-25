import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Controls.Material
import FishingEngine

Window {
    width: 480
    height: 800
    visible: true
    title: "Fishing Engine v2.0"

    color: "#F8FAFC"

    Material.theme: Material.Light
    Material.accent: "#0EA5E9"
    Material.primary: "#0284C7"

    // --- State Properties ---
    property bool isCalculated: false

    property string mainScoreText: "--"
    property string mainScoreColor: "#94A3B8"
    property string arrowColor: "#94A3B8"

    property string surfaceScoreStr: "--"
    property string depthScoreStr: "--"
    property string depthLabelStr: "ΙΔΑΝΙΚΟ ΒΑΘΟΣ"

    property string thermoclineInfo: "Αναμονή δεδομένων..."
    property string tempText: "--"
    property string windText: "--"
    property string rainText: "--"
    property string pressureText: "--"

    function getScoreColor(score) {
        if (score >= 0.8) return "#22C55E";
        if (score >= 0.6) return "#84CC16";
        if (score >= 0.4) return "#EAB308";
        if (score >= 0.2) return "#F97316";
        return "#EF4444";
    }

    EngineController {
        id: backend

        onCalculationFinished: function(surfacePct, thermoPct, thermoDepth, stats) {
            isCalculated = true;

            let bestPct = Math.max(surfacePct, thermoPct);
            mainScoreText = bestPct.toFixed(1) + "%";
            mainScoreColor = getScoreColor(bestPct / 100.0);
            arrowColor = getScoreColor(stats.scoreWindDir);

            surfaceScoreStr = surfacePct.toFixed(1) + "%";

            if (thermoDepth > 0) {
                depthScoreStr = thermoPct.toFixed(1) + "%";
                depthLabelStr = `ΒΑΘΟΣ (${stats.bestDepth.toFixed(0)}m)`;
                thermoclineInfo = `Στρώμα Επιλιμνίου: 0 έως ${stats.thermoclineDepth.toFixed(1)}m`;
            } else {
                depthScoreStr = surfacePct.toFixed(1) + "%";
                depthLabelStr = "ΟΛΑ ΤΑ ΒΑΘΗ";
                thermoclineInfo = "Η λίμνη είναι πλήρως ανακατεμένη (Turnover)";
            }

            let waterHtml = `Νερό: <font color='${getScoreColor(stats.scoreTemp)}'><b>${stats.surfaceTemp.toFixed(1)}°C</b></font>`;
            let airHtml = `<font size='2' color='#64748B'>Αέρας: ${stats.airTemp.toFixed(1)}°C</font>`;
            tempText = waterHtml + "<br>" + airHtml;

            windText = `<b>${stats.beaufort} Bft</b><br><font size='2' color='#64748B'>${stats.windKmh.toFixed(1)} km/h</font>`;
            rainText = `<font color='${getScoreColor(stats.scoreRain)}'><b>${stats.rain.toFixed(1)} mm</b></font>`;
            pressureText = `<font color='${getScoreColor(stats.scorePressure)}'><b>${stats.pressure.toFixed(1)} hPa</b></font>`;
        }

        onCalculationError: function(errorMessage) {
            isCalculated = false;
            thermoclineInfo = `<font color='#EF4444'><b>Σφάλμα:</b> ${errorMessage}</font>`;
        }
    }

    // --- MAIN LAYOUT ΜΕ SCROLLING (ΚΥΛΙΣΗ) ---
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth // Απενεργοποιεί το οριζόντιο scrolling
        clip: true // Κρύβει το περιεχόμενο που βγαίνει εκτός οθόνης
        ScrollBar.vertical.policy: ScrollBar.AsNeeded // Εμφανίζει τη μπάρα μόνο αν χρειάζεται

        ColumnLayout {
            // Ορίζουμε το πλάτος λίγο μικρότερο για να αφήσουμε περιθώρια (margins) δεξιά-αριστερά
            width: parent.width - 48
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 24

            Item { Layout.preferredHeight: 10 } // Spacer (Κενό στην κορυφή)

            // 1. HEADER
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                spacing: 4

                Text {
                    text: "🐟 Fishing Engine"
                    font.pixelSize: 32
                    font.bold: true
                    color: "#0F172A"
                    Layout.alignment: Qt.AlignHCenter
                }
                Text {
                    text: "Advanced Simulation Model"
                    font.pixelSize: 14
                    color: "#64748B"
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // 2. ΚΑΡΤΑ ΕΠΙΛΟΓΩΝ
            Pane {
                Layout.fillWidth: true
                Material.elevation: 2
                padding: 24

                background: Rectangle { color: "white"; radius: 16 }

                ColumnLayout {
                    width: parent.width
                    spacing: 16

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 4
                        Text { text: "ΤΟΠΟΘΕΣΙΑ"; font.pixelSize: 12; font.bold: true; color: "#94A3B8" }
                        ComboBox {
                            id: lakeCombo; Layout.fillWidth: true; font.pixelSize: 16
                            textRole: "text"; valueRole: "value"
                            model: ListModel {
                                ListElement { text: "Τριχωνίδα (Δυτικά)"; value: "trichonida_west" }
                                ListElement { text: "Τριχωνίδα (Ανατολικά)"; value: "trichonida_east" }
                                ListElement { text: "Ρίβιο"; value: "rivio" }
                                ListElement { text: "Οζερός"; value: "ozeros" }
                                ListElement { text: "Βουλκαρία"; value: "voulkaria" }
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 4
                        Text { text: "ΣΤΟΧΟΣ"; font.pixelSize: 12; font.bold: true; color: "#94A3B8" }
                        ComboBox {
                            id: fishCombo; Layout.fillWidth: true; font.pixelSize: 16
                            textRole: "text"; valueRole: "value"
                            model: ListModel {
                                ListElement { text: "Γριβάδι (Carp)"; value: "carp" }
                                ListElement { text: "Πεταλούδα"; value: "petalouda" }
                            }
                        }
                    }

                    Button {
                        text: "ΑΝΑΛΥΣΗ ΣΥΝΘΗΚΩΝ"
                        Layout.fillWidth: true; Layout.topMargin: 10; Layout.preferredHeight: 50
                        font.pixelSize: 15; font.bold: true
                        Material.background: Material.accent; Material.foreground: "white"; Material.roundedScale: Material.MediumScale
                        onClicked: {
                            isCalculated = false;
                            thermoclineInfo = "Λήψη μετεωρολογικών δεδομένων...";
                            backend.calculateCatchProbability(lakeCombo.currentValue, fishCombo.currentValue)
                        }
                    }
                }
            }

            // 3. ΠΕΡΙΟΧΗ ΑΠΟΤΕΛΕΣΜΑΤΩΝ
            ColumnLayout {
                Layout.fillWidth: true
                // ΑΦΑΙΡΕΘΗΚΕ: το Layout.fillHeight: true για να μακραίνει δυναμικά!
                spacing: 20
                opacity: isCalculated ? 1.0 : 0.0
                visible: opacity > 0

                Behavior on opacity { NumberAnimation { duration: 400; easing.type: Easing.InOutQuad } }

                // Κεντρικό Σκορ
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 15

                    Text {
                        text: "↑"
                        font.pixelSize: 58
                        font.bold: true
                        color: arrowColor
                        rotation: backend.windDegrees + 180
                        Behavior on rotation { NumberAnimation { duration: 1000; easing.type: Easing.OutElastic } }
                    }

                    Text {
                        text: mainScoreText
                        font.pixelSize: 58
                        font.bold: true
                        color: mainScoreColor
                    }
                }

                // Premium Κάψουλα για το Split View
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    color: "#F0F9FF"
                    radius: 16
                    border.color: "#E0F2FE"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: "ΕΠΙΦΑΝΕΙΑ"; font.pixelSize: 11; font.bold: true; color: "#7DD3FC"; Layout.alignment: Qt.AlignHCenter }
                            Text { text: surfaceScoreStr; font.pixelSize: 20; font.bold: true; color: "#0284C7"; Layout.alignment: Qt.AlignHCenter }
                        }

                        Rectangle { width: 1; height: 35; color: "#BAE6FD" }

                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: depthLabelStr; font.pixelSize: 11; font.bold: true; color: "#7DD3FC"; Layout.alignment: Qt.AlignHCenter }
                            Text { text: depthScoreStr; font.pixelSize: 20; font.bold: true; color: "#0284C7"; Layout.alignment: Qt.AlignHCenter }
                        }
                    }
                }

                Text {
                    text: thermoclineInfo
                    font.pixelSize: 13
                    color: "#64748B"
                    Layout.alignment: Qt.AlignHCenter
                    textFormat: Text.RichText
                }

                // Αναβαθμισμένο StatCard (με Εικονίδιο & Σκιά)
                GridLayout {
                    columns: 2
                    Layout.fillWidth: true
                    columnSpacing: 14
                    rowSpacing: 14
                    Layout.topMargin: 5

                    component StatCard : Pane {
                        id: cardRoot
                        property string iconStr: ""
                        property string titleText: ""
                        property string valueText: ""

                        Layout.fillWidth: true
                        Layout.preferredHeight: 85
                        Material.elevation: 1
                        padding: 14

                        background: Rectangle { color: "white"; radius: 16 }

                        RowLayout {
                            anchors.fill: parent
                            spacing: 12

                            Text { text: cardRoot.iconStr; font.pixelSize: 26; Layout.alignment: Qt.AlignVCenter }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 4
                                Text { text: cardRoot.titleText; font.pixelSize: 10; color: "#94A3B8"; font.bold: true; }
                                Text { text: cardRoot.valueText; font.pixelSize: 14; color: "#334155"; textFormat: Text.RichText; }
                            }
                        }
                    }

                    StatCard { iconStr: "🌡️"; titleText: "ΘΕΡΜΟΚΡΑΣΙΕΣ"; valueText: tempText }
                    StatCard { iconStr: "💨"; titleText: "ΑΝΕΜΟΣ (Live)"; valueText: windText }
                    StatCard { iconStr: "🌧️"; titleText: "ΒΡΟΧΗ (ΗΜΕΡΗΣΙΑ)"; valueText: rainText }
                    StatCard { iconStr: "🧭"; titleText: "ΒΑΡΟΜΕΤΡΟ"; valueText: pressureText }
                }
            }

            Item { Layout.preferredHeight: 30 } // Spacer (Κενό στο τέλος για να μη "κολλάει" η κάρτα στον πάτο)
        }
    }
}