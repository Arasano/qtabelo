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

#ifndef TABLE_DOCUMENT_H
#define TABLE_DOCUMENT_H

#include "qtablewidget.h"
#include <QWidget>

#include <QTabWidget>


class TableDocument : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool tabBarVisible MEMBER m_tabBarVisible READ isTabBarVisible WRITE setTabBarVisible RESET resetTabBarVisible NOTIFY tabBarVisibleChanged)
    Q_PROPERTY(QTabWidget::TabPosition tabBarPosition READ tabBarPosition WRITE setTabBarPosition RESET resetTabBarPosition NOTIFY tabBarPositionChanged)
    Q_PROPERTY(bool tabBarAutoHide READ isTabBarAutoHide WRITE setTabBarAutoHide RESET resetTabBarAutoHide NOTIFY tabBarAutoHideChanged)

public:
    explicit TableDocument(QWidget *parent = nullptr);

    void saveSettings();

    bool isTabBarVisible() const;
    QTabWidget::TabPosition tabBarPosition() const;
    bool isTabBarAutoHide() const;

signals:
    void tabBarVisibleChanged(const bool visible);
    void tabBarPositionChanged(const QTabWidget::TabPosition position);
    void tabBarAutoHideChanged(const bool enabled);

    void contentChanged(bool modified);

public slots:
    void setTabBarVisible(const bool visible);
    void resetTabBarVisible();
    void initTabBarVisible();

    void setTabBarPosition(const QTabWidget::TabPosition position);
    void resetTabBarPosition();
    void initTabBarPosition();

    void setTabBarAutoHide(const bool enabled);
    void resetTabBarAutoHide();
    void initTabBarAutoHide();

    void showTableContextMenu(QTableWidget *table, const QPoint &pos);

protected slots:
    void slotAddTab(const int count);

private:
    void loadSettings();

    void _setTabBarVisible(const bool visible);

private slots:
    void slotCloseTab(const int index);

private:
    QTabWidget *m_tabs;

    bool m_tabBarVisible;
};

#endif // TABLE_DOCUMENT_H
