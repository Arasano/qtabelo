#ifndef TABLE_INFO_PROVIDER_H
#define TABLE_INFO_PROVIDER_H

#include <QObject>

class TableInfoProvider : public QObject
{
    Q_OBJECT

    // Три свойства, которые будут видны из QML
    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(int columnCount READ columnCount NOTIFY columnCountChanged)
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)

public:
    explicit TableInfoProvider(QObject *parent = nullptr);

    int rowCount() const;
    int columnCount() const;
    bool isModified() const;

public slots:
    void setRowCount(int rows);
    void setColumnCount(int columns);
    void setModified(bool modified);

signals:
    void rowCountChanged();
    void columnCountChanged();
    void modifiedChanged();

private:
    int m_rowCount;
    int m_columnCount;
    bool m_modified;
};

#endif // TABLE_INFO_PROVIDER_H
