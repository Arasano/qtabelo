#include "table_info_provider.h"

TableInfoProvider::TableInfoProvider(QObject *parent)
    : QObject(parent)
    , m_rowCount{0}
    , m_columnCount{0}
    , m_modified{false}
    , m_cellAddress{}
    , m_cellValue{}
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


QString TableInfoProvider::cellAddress() const
{
    return m_cellAddress;
}

QString TableInfoProvider::cellValue() const
{
    return m_cellValue;
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

void TableInfoProvider::setCellAddress(const QString &address)
{
    if (address != m_cellAddress) {
        m_cellAddress = address;
        emit cellAddressChanged();
    }
}

void TableInfoProvider::setCellValue(const QString &value)
{
    if (value != m_cellValue) {
        m_cellValue = value;
        emit cellValueChanged();
    }
}
