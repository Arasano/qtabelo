#include "table_document.h"
#include "custom_table_model.h"
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QItemSelectionModel>
#include <QAbstractItemModel>
#include <QSettings>
#include <QTabBar>
#include <QVBoxLayout>
#include <QDebug>

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
    const bool visible = settings.value(QStringLiteral("Document/SheetTabBarVisible"), true).toBool();
    setTabBarVisible(visible);

    const int value = settings.value(QStringLiteral("Document/SheetTabBarPosition"), QTabWidget::South).toInt();
    const QList<int> values = {QTabWidget::North, QTabWidget::South};
    const QTabWidget::TabPosition position = values.contains(value) ? static_cast<QTabWidget::TabPosition>(value) : QTabWidget::South;
    setTabBarPosition(position);

    const bool enabled = settings.value(QStringLiteral("Document/SheetTabBarAutoHide"), true).toBool();
    setTabBarAutoHide(enabled);
}

void TableDocument::saveSettings()
{
    QSettings settings;
    // settings.setValue(QStringLiteral("Document/SheetTabBarVisible"), isTabBarVisible());
    // settings.setValue(QStringLiteral("Document/SheetTabBarPosition"), tabBarPosition());
    // settings.setValue(QStringLiteral("Document/SheetTabBarAutoHide"), isTabBarAutoHide());
}


bool TableDocument::isTabBarVisible() const { return m_tabBarVisible; }

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


QTabWidget::TabPosition TableDocument::tabBarPosition() const { return m_tabs->tabPosition(); }

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


bool TableDocument::isTabBarAutoHide() const { return m_tabs->tabBarAutoHide(); }

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

// === slotAddTab: creates QTableView + CustomTableModel ===
void TableDocument::slotAddTab(const int count)
{
    if (m_tabs->count() == 0) {
        for (int i = 0; i < count; ++i) {
            // create view
            auto *tableView = new QTableView();
            tableView->setObjectName(QStringLiteral("Sheet%1").arg(i + 1));

            // create model & link to view
            auto *model = new CustomTableModel(tableView);
            tableView->setModel(model);

            // signals for QML-panel
            connect(tableView->selectionModel(), &QItemSelectionModel::currentChanged,
                    this, [this]() { refreshCurrentCell(); });
            connect(model, &QAbstractItemModel::dataChanged,
                    this, [this]() { refreshCurrentCell(); });

            // initialize blank table 15x5
            QStringList headers;
            for (int col = 0; col < 5; ++col) {
                headers << "";
            }
            model->initialize(15, 5, headers);

            // context menu
            tableView->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(tableView, &QTableView::customContextMenuRequested,
                    this, [this, tableView](const QPoint &pos) {
                        showTableContextMenu(tableView, pos);
                    });

            // data changed signals
            connect(model, &QAbstractItemModel::dataChanged,
                    this, &TableDocument::onModelDataChanged);

            // add tab
            m_tabs->addTab(tableView, tr("Sheet %1").arg(i + 1));
        }
    }
}

void TableDocument::onModelDataChanged()
{
    emit contentChanged(true);
}


void TableDocument::showTableContextMenu(QTableView *tableView, const QPoint &pos)
{
    if (!tableView)
        return;

    auto *model = qobject_cast<CustomTableModel*>(tableView->model());
    if (!model)
        return;

    QMenu menu;
    QAction *actionAddRow = menu.addAction(tr("Добавить строку"));
    QAction *actionRemoveRow = menu.addAction(tr("Удалить строку"));
    QAction *actionAddColumn = menu.addAction(tr("Добавить столбец"));
    QAction *actionRemoveColumn = menu.addAction(tr("Удалить столбец"));
    menu.addSeparator();
    QAction *actionCopy = menu.addAction(tr("Копировать"));
    QAction *actionCut = menu.addAction(tr("Вырезать"));
    QAction *actionPaste = menu.addAction(tr("Вставить"));

    QAction *selectedAction = menu.exec(tableView->mapToGlobal(pos));
    if (!selectedAction)
        return;

    // get current cursor position
    QModelIndex currentIndex = tableView->currentIndex();
    int currentRow = currentIndex.isValid() ? currentIndex.row() : 0;
    int currentCol = currentIndex.isValid() ? currentIndex.column() : 0;

    if (selectedAction == actionAddRow) {
        model->insertRows(currentRow + 1, 1);
        emit contentChanged(true);
    }
    else if (selectedAction == actionRemoveRow && model->rowCount() > 0) {
        int rowToRemove = currentRow < model->rowCount() ? currentRow : model->rowCount() - 1;
        model->removeRows(rowToRemove, 1);
        emit contentChanged(true);
    }
    else if (selectedAction == actionAddColumn) {
        model->insertColumns(currentCol + 1, 1);
        emit contentChanged(true);
    }
    else if (selectedAction == actionRemoveColumn && model->columnCount() > 0) {
        int colToRemove = currentCol < model->columnCount() ? currentCol : model->columnCount() - 1;
        model->removeColumns(colToRemove, 1);
        emit contentChanged(true);
    }
    else if (selectedAction == actionCopy) {
        copySelection(tableView);
    }
    else if (selectedAction == actionCut) {
        cutSelection(tableView);
    }
    else if (selectedAction == actionPaste) {
        pasteSelection(tableView);
    }
}


void TableDocument::copySelection(QTableView *tableView)
{
    if (!tableView || !tableView->selectionModel())
        return;

    auto *model = tableView->model();
    if (!model)
        return;

    QModelIndexList selected = tableView->selectionModel()->selectedIndexes();
    if (selected.isEmpty())
        return;

    // get highlight borders
    int minRow = model->rowCount();
    int maxRow = 0;
    int minCol = model->columnCount();
    int maxCol = 0;

    for (const QModelIndex &index : selected) {
        minRow = qMin(minRow, index.row());
        maxRow = qMax(maxRow, index.row());
        minCol = qMin(minCol, index.column());
        maxCol = qMax(maxCol, index.column());
    }

    QString clipboardText;
    for (int row = minRow; row <= maxRow; ++row) {
        QStringList rowText;
        for (int col = minCol; col <= maxCol; ++col) {
            QModelIndex idx = model->index(row, col);
            rowText << model->data(idx, Qt::DisplayRole).toString();
        }
        clipboardText += rowText.join('\t') + '\n';
    }

    QApplication::clipboard()->setText(clipboardText);
}

void TableDocument::cutSelection(QTableView *tableView)
{
    if (!tableView || !tableView->selectionModel())
        return;

    auto *model = tableView->model();
    if (!model)
        return;

    // copy strings
    copySelection(tableView);

    // clean highlight strings
    QModelIndexList selected = tableView->selectionModel()->selectedIndexes();
    for (const QModelIndex &index : selected) {
        model->setData(index, QVariant(), Qt::EditRole);
    }

    emit contentChanged(true);
}

void TableDocument::pasteSelection(QTableView *tableView)
{
    if (!tableView)
        return;

    auto *model = qobject_cast<CustomTableModel*>(tableView->model());
    if (!model)
        return;

    QString clipboardText = QApplication::clipboard()->text();
    if (clipboardText.isEmpty())
        return;

    QStringList rows = clipboardText.split('\n', Qt::SkipEmptyParts);
    if (rows.isEmpty())
        return;

    int pasteRows = rows.count();
    int pasteCols = 0;
    for (const QString &row : rows) {
        pasteCols = qMax(pasteCols, row.split('\t').count());
    }

    int startRow = 0;
    int startCol = 0;
    QModelIndex currentIndex = tableView->currentIndex();
    if (currentIndex.isValid()) {
        startRow = currentIndex.row();
        startCol = currentIndex.column();
    }

    int requiredRows = startRow + pasteRows;
    int requiredCols = startCol + pasteCols;

    if (requiredRows > model->rowCount()) {
        model->insertRows(model->rowCount(), requiredRows - model->rowCount());
    }
    if (requiredCols > model->columnCount()) {
        model->insertColumns(model->columnCount(), requiredCols - model->columnCount());
    }

    for (int i = 0; i < pasteRows; ++i) {
        QStringList cells = rows[i].split('\t');
        for (int j = 0; j < cells.count() && j < pasteCols; ++j) {
            QModelIndex idx = model->index(startRow + i, startCol + j);
            model->setData(idx, cells[j], Qt::EditRole);
        }
    }

    emit contentChanged(true);
}

void TableDocument::slotCloseTab(const int index)
{
    if (m_tabs->count() > 1) {
        QWidget *widget = m_tabs->widget(index);
        m_tabs->removeTab(index);

        if (widget) {
            if (auto *tableView = qobject_cast<QTableView*>(widget)) {
                tableView->setModel(nullptr);
            }
            widget->deleteLater();
        }

        m_tabs->setTabsClosable(m_tabs->count() > 1);
    }
}

// clean up all models by closing
void TableDocument::cleanupModels()
{
    for (int i = 0; i < m_tabs->count(); ++i) {
        if (auto *tableView = qobject_cast<QTableView*>(m_tabs->widget(i))) {
            if (auto *model = tableView->model()) {
                disconnect(model, nullptr, this, nullptr);
                tableView->setModel(nullptr);
            }
        }
    }
}

void TableDocument::refreshCurrentCell()
{
    QTabWidget *tabs = findChild<QTabWidget*>();
    QTableView *view = tabs ? qobject_cast<QTableView*>(tabs->currentWidget()) : nullptr;

    if (!view || !view->selectionModel() || !view->selectionModel()->currentIndex().isValid()) {
        emit currentCellChanged(QString(), QString());
        return;
    }

    const QModelIndex cur = view->selectionModel()->currentIndex();

    QString colHeader = view->model()->headerData(cur.column(), Qt::Horizontal, Qt::DisplayRole).toString();

    const QString address = QString("%1, строка %2").arg(colHeader).arg(cur.row() + 1);

    const QString value = view->model()->data(cur, Qt::DisplayRole).toString();

    emit currentCellChanged(address, value);
}
