#ifndef RESOURCEMODEL_H
#define RESOURCEMODEL_H

#include <QAbstractItemModel>

class ResourceModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit ResourceModel(QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex	parent(const QModelIndex & index) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const ;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
signals:
    
public slots:
    
};

#endif // RESOURCEMODEL_H
