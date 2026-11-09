#ifndef TABLE_INFO_PROVIDER_H
#define TABLE_INFO_PROVIDER_H

#include <QObject>

class TableInfoProvider : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int rowCount READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(int columnCount READ columnCount NOTIFY columnCountChanged)
    Q_PROPERTY(bool modified READ isModified NOTIFY modifiedChanged)

    Q_PROPERTY(QString cellAddress READ cellAddress WRITE setCellAddress NOTIFY cellAddressChanged)
    Q_PROPERTY(QString cellValue READ cellValue WRITE setCellValue NOTIFY cellValueChanged)

public:
    explicit TableInfoProvider(QObject *parent = nullptr);

    int rowCount() const;
    int columnCount() const;
    bool isModified() const;

    QString cellAddress() const;
    QString cellValue() const;

public slots:
    void setRowCount(int rows);
    void setColumnCount(int columns);
    void setModified(bool modified);

    void setCellAddress(const QString &address);
    void setCellValue(const QString &value);

signals:
    void rowCountChanged();
    void columnCountChanged();
    void modifiedChanged();

    void cellAddressChanged();
    void cellValueChanged();

private:
    int m_rowCount;
    int m_columnCount;
    bool m_modified;
    QString m_cellAddress;
    QString m_cellValue;
};
#endif // TABLE_INFO_PROVIDER_H
