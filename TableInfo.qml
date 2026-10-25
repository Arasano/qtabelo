import QtQuick 2.15

Rectangle {
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
        }
    }
}
