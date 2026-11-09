#include "custom_table_model.h"

CustomTableModel::CustomTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int CustomTableModel::rowCount(const QModelIndex &parent) const
{
    // for flat table parent havent to had child elems
    if (parent.isValid())
        return 0;
    return m_data.size();
}

int CustomTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_headers.isEmpty() ? 0 : m_headers.size();
}

QVariant CustomTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_data.size() || index.column() >= m_headers.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return m_data.at(index.row()).at(index.column());
    }

    return QVariant();
}

bool CustomTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {

        m_data[index.row()][index.column()] = value.toString();

        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

QVariant CustomTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {

            if (section < m_headers.size() && !m_headers.at(section).isEmpty()) {
                return m_headers.at(section);
            }
            return QString("Column %1").arg(section + 1);
        } else {
            return section + 1;
        }
    }
    return QVariant();
}

Qt::ItemFlags CustomTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

bool CustomTableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        m_data.insert(row, QStringList(m_headers.size(), ""));
    }
    endInsertRows();
    return true;
}

bool CustomTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        m_data.removeAt(row);
    }
    endRemoveRows();
    return true;
}

bool CustomTableModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginInsertColumns(QModelIndex(), column, column + count - 1);
    for (int i = 0; i < count; ++i) {

        m_headers.insert(column + i, "");
        for (int row = 0; row < m_data.size(); ++row) {
            m_data[row].insert(column + i, "");
        }
    }
    endInsertColumns();
    return true;
}

bool CustomTableModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent)
    beginRemoveColumns(QModelIndex(), column, column + count - 1);
    for (int i = 0; i < count; ++i) {
        m_headers.removeAt(column);
        for (int row = 0; row < m_data.size(); ++row) {
            m_data[row].removeAt(column);
        }
    }
    endRemoveColumns();
    return true;
}


void CustomTableModel::initialize(int rows, int columns, const QStringList &headers)
{
    beginResetModel();  // full model reset
    m_headers = headers;
    m_data.clear();
    m_data.reserve(rows);
    for (int i = 0; i < rows; ++i) {
        m_data.append(QStringList(columns, ""));
    }
    endResetModel();
}

void CustomTableModel::loadFromCsv(const QStringList &headers, const QList<QStringList> &rows)
{
    beginResetModel();
    m_headers = headers;
    m_data = rows;
    endResetModel();
}
