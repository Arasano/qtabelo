/**
 * Copyright 2022 naracanto <https://naracanto.github.io>.
 *
 * This file is part of QTabelo <https://github.com/beletalabs/qtabelo>.
 *
 * QTabelo is an open source table editor written in C++ using the
 * Qt framework.
 *
 * QTabelo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * QTabelo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with QTabelo.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "table_document.h"
#include "qmenu.h"

#include <QSettings>
#include <QTabBar>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QMenu>
#include <QAction>
#include <QTableWidgetItem>


TableDocument::TableDocument(QWidget *parent)
    : QWidget(parent)
    , m_tabs{new QTabWidget}
    , m_tabBarVisible{true}
{
    m_tabs->setDocumentMode(true);
    m_tabs->setMovable(true);
    m_tabs->setTabPosition(QTabWidget::South);
    m_tabs->setTabBarAutoHide(true);
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &TableDocument::slotCloseTab);

    loadSettings();

    // Main layout
    auto *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(m_tabs);
    setLayout(mainLayout);
}


void TableDocument::loadSettings()
{
    QSettings settings;

    // Sheet Tab Bar Visible
    const bool visible = settings.value(QStringLiteral("Document/SheetTabBarVisible"), true).toBool();
    setTabBarVisible(visible);

    // Sheet Tab Bar Position
    const int value = settings.value(QStringLiteral("Document/SheetTabBarPosition"), QTabWidget::South).toInt();
    const QList<int> values = {QTabWidget::North, QTabWidget::South};
    const QTabWidget::TabPosition position = values.contains(value) ? static_cast<QTabWidget::TabPosition>(value) : QTabWidget::South;
    setTabBarPosition(position);

    // Sheet Tab Bar Auto Hide
    const bool enabled = settings.value(QStringLiteral("Document/SheetTabBarAutoHide"), true).toBool();
    setTabBarAutoHide(enabled);
}


void TableDocument::saveSettings()
{
    QSettings settings;

    // Sheet Tab Bar Visible
//    const bool visible = isTabBarVisible();
//    settings.setValue(QStringLiteral("Document/SheetTabBarVisible"), visible);

    // Sheet Tab Bar Position
//    const QTabWidget::TabPosition position = tabBarPosition();
//    settings.setValue(QStringLiteral("Document/SheetTabBarPosition"), position);

    // Sheet Tab Bar Auto Hide
//    const bool enabled = isTabBarAutoHide();
//    settings.setValue(QStringLiteral("Document/SheetTabBarAutoHide"), enabled);
}


//
// Property: tabBarVisible
//

bool TableDocument::isTabBarVisible() const
{
    return m_tabBarVisible;
}


void TableDocument::setTabBarVisible(const bool visible)
{
    if (visible != isTabBarVisible()) {
        m_tabBarVisible = visible;
        _setTabBarVisible(isTabBarVisible());
        emit tabBarVisibleChanged(isTabBarVisible());
    }
}


void TableDocument::resetTabBarVisible()
{
    m_tabBarVisible = true;
    _setTabBarVisible(isTabBarVisible());
    emit tabBarVisibleChanged(isTabBarVisible());
}


void TableDocument::initTabBarVisible()
{
    _setTabBarVisible(isTabBarVisible());
    emit tabBarVisibleChanged(isTabBarVisible());
}


void TableDocument::_setTabBarVisible(const bool visible)
{
    if (!(m_tabs->count() <= 1 && m_tabs->tabBarAutoHide()))
        m_tabs->tabBar()->setVisible(visible);
}


//
// Property: tabBarPosition
//

QTabWidget::TabPosition TableDocument::tabBarPosition() const
{
    return m_tabs->tabPosition();
}


void TableDocument::setTabBarPosition(const QTabWidget::TabPosition position)
{
    if (position != tabBarPosition()) {
        m_tabs->setTabPosition(position);
        emit tabBarPositionChanged(tabBarPosition());
    }
}


void TableDocument::resetTabBarPosition()
{
    m_tabs->setTabPosition(QTabWidget::South);
    emit tabBarPositionChanged(tabBarPosition());
}


void TableDocument::initTabBarPosition()
{
    emit tabBarPositionChanged(tabBarPosition());
}


//
// Property: tabBarAutoHide
//

bool TableDocument::isTabBarAutoHide() const
{
    return m_tabs->tabBarAutoHide();
}


void TableDocument::setTabBarAutoHide(const bool enabled)
{
    if (enabled != isTabBarAutoHide()) {
        m_tabs->setTabBarAutoHide(enabled);
        emit tabBarAutoHideChanged(isTabBarAutoHide());
    }
}


void TableDocument::resetTabBarAutoHide()
{
    m_tabs->setTabBarAutoHide(true);
    emit tabBarAutoHideChanged(isTabBarAutoHide());
}


void TableDocument::initTabBarAutoHide()
{
    emit tabBarAutoHideChanged(isTabBarAutoHide());
}


//
// Slots
//

void TableDocument::showTableContextMenu(QTableWidget *table, const QPoint &pos)
{
    QMenu menu;

    QAction *actionAddRow = menu.addAction(tr("Добавить строку"));
    QAction *actionRemoveRow = menu.addAction(tr("Удалить последнюю строку"));
    QAction *actionAddColumn = menu.addAction(tr("Добавить столбец"));
    QAction *actionRemoveColumn = menu.addAction(tr("Удалить последний столбец"));

    QAction *selectedAction = menu.exec(table->mapToGlobal(pos));

    if (!selectedAction)
        return;

    if (selectedAction == actionAddRow) {
        table->setRowCount(table->rowCount() + 1);
        emit contentChanged(true); // Уведомляем об изменении
    }
    else if (selectedAction == actionRemoveRow && table->rowCount() > 0) {
        table->setRowCount(table->rowCount() - 1);
        emit contentChanged(true);
    }
    else if (selectedAction == actionAddColumn) {
        int newColIndex = table->columnCount();
        table->setColumnCount(newColIndex + 1);
        table->setHorizontalHeaderItem(newColIndex, new QTableWidgetItem(QString("Столбец %1").arg(newColIndex + 1)));
        emit contentChanged(true);
    }
    else if (selectedAction == actionRemoveColumn && table->columnCount() > 0) {
        table->setColumnCount(table->columnCount() - 1);
        emit contentChanged(true);
    }
}

void TableDocument::slotAddTab(int count)
{
    if (m_tabs->count() == 0) {
        for (int i = 0; i < count; ++i) {
            QTableWidget *table = new QTableWidget(10, 5, this);
            table->setAttribute(Qt::WA_DeleteOnClose);
            table->setContextMenuPolicy(Qt::CustomContextMenu);

            // Устанавливаем заголовки столбцов
            QStringList headers;
            for (int col = 0; col < 5; ++col) {
                headers << QString("Столбец %1").arg(col + 1);
            }
            table->setHorizontalHeaderLabels(headers);

            // Подключаем контекстное меню
            connect(table, &QTableWidget::customContextMenuRequested,
                    this, [this, table](const QPoint &pos) {
                        showTableContextMenu(table, pos);
                    });

            // Подключаем сигнал изменения ячеек
            connect(table, &QTableWidget::itemChanged,
                    this, [this](QTableWidgetItem *item) {
                        Q_UNUSED(item);
                        emit contentChanged(true);
                    });

            m_tabs->addTab(table, tr("Лист %1").arg(i + 1));
        }
    }
}
void TableDocument::slotCloseTab(const int index)
{
    if (m_tabs->count() > 1) {

        auto widget = m_tabs->widget(index);
        if (widget) {
            widget->close();
            m_tabs->removeTab(index);
        }

        m_tabs->setTabsClosable(m_tabs->count() > 1);
    }
}
