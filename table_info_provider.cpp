#include "table_info_provider.h"

TableInfoProvider::TableInfoProvider(QObject *parent)
    : QObject(parent)
    , m_rowCount{0}
    , m_columnCount{0}
    , m_modified{false}
{
}

int TableInfoProvider::rowCount() const
{
    return m_rowCount;
}

int TableInfoProvider::columnCount() const
{
    return m_columnCount;
}

bool TableInfoProvider::isModified() const
{
    return m_modified;
}

void TableInfoProvider::setRowCount(int rows)
{
    if (rows != m_rowCount) {
        m_rowCount = rows;
        emit rowCountChanged();
    }
}

void TableInfoProvider::setColumnCount(int columns)
{
    if (columns != m_columnCount) {
        m_columnCount = columns;
        emit columnCountChanged();
    }
}

void TableInfoProvider::setModified(bool modified)
{
    if (modified != m_modified) {
        m_modified = modified;
        emit modifiedChanged();
    }
}
