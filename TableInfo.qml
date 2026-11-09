import QtQuick 2.15

Rectangle {
    id: root
    signal saveRequested()

    color: "#f5f5f5"
    border.color: "#cccccc"
    border.width: 1

    Column {
        anchors.centerIn: parent
        spacing: 12

        Text {
            text: "Информация о таблице"
            font.bold: true
            font.pixelSize: 16
            color: "#333333"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            width: parent.width
            height: 1
            color: "#cccccc"
        }

        Text {
            text: "Строк: " + tableInfo.rowCount
            font.pixelSize: 14
            color: "#444444"
        }

        Text {
            text: "Столбцов: " + tableInfo.columnCount
            font.pixelSize: 14
            color: "#444444"
        }

        Text {
            text: tableInfo.modified ? "Состояние: Изменён" : "Состояние: Сохранён"
            font.pixelSize: 14
            font.bold: true
            color: tableInfo.modified ? "#cc0000" : "#008800"

            // Плавная анимация смены цвета
            Behavior on color {
                ColorAnimation {
                    duration: 400
                    easing.type: Easing.InOutQuad
                }
            }
        }

        // Индикатор текущей ячейки
        Text {
            text: tableInfo.cellAddress !== "" ? "Ячейка: " + tableInfo.cellAddress
                                               : "Ячейка: не выделена"
            font.pixelSize: 13
            font.bold: true
            color: "#444444"
        }

        Text {
            width: parent.width - 20
            text: tableInfo.cellValue !== "" ? tableInfo.cellValue : "(пусто)"
            font.pixelSize: 12
            color: "#555555"
            wrapMode: Text.WrapAnywhere
            maximumLineCount: 3
            elide: Text.ElideRight
        }

        // Кнопка "Сохранить"
        Rectangle {
            id: saveButton
            width: 120
            height: 32
            radius: 4
            color: saveArea.pressed ? "#a8a8a8" : "#d8d8d8"
            anchors.horizontalCenter: parent.horizontalCenter

            Text {
                anchors.centerIn: parent
                text: "Сохранить"
                font.pixelSize: 13
            }

            MouseArea {
                id: saveArea
                anchors.fill: parent
                onClicked: root.saveRequested()
            }
        }
    }
}
