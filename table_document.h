#ifndef TABLE_DOCUMENT_H
#define TABLE_DOCUMENT_H

#include <QWidget>
#include <QTabWidget>
#include <QTableView>

class CustomTableModel;

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
    void cleanupModels();
    void refreshCurrentCell();

signals:
    void tabBarVisibleChanged(const bool visible);
    void tabBarPositionChanged(const QTabWidget::TabPosition position);
    void tabBarAutoHideChanged(const bool enabled);
    void contentChanged(bool modified);
    void currentCellChanged(const QString &address, const QString &value);

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
    void showTableContextMenu(QTableView *tableView, const QPoint &pos);
    void onModelDataChanged();
    void slotAddTab(const int count);

private:
    void loadSettings();
    void _setTabBarVisible(const bool visible);
    void copySelection(QTableView *tableView);
    void cutSelection(QTableView *tableView);
    void pasteSelection(QTableView *tableView);

private slots:
    void slotCloseTab(const int index);

private:
    QTabWidget *m_tabs;
    bool m_tabBarVisible;
};

#endif // TABLE_DOCUMENT_H
