#ifndef NAVIGATORMODEL_H
#define NAVIGATORMODEL_H

#include <QAbstractItemModel>

class NavigatorModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit NavigatorModel(QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex	parent(const QModelIndex & index) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const ;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
signals:
    
public slots:
    
};

#endif // NAVIGATORMODEL_H
