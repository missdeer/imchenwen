#include "navigatormodel.h"

NavigatorModel::NavigatorModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

QModelIndex NavigatorModel::index(int row, int column, const QModelIndex &parent) const
{
    return QModelIndex();
}

QModelIndex NavigatorModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int NavigatorModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

int NavigatorModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant NavigatorModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QVariant NavigatorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole)
        return QVariant(tr("Service"));
    return QVariant();
}
