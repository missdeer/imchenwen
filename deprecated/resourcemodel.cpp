#include "resourcemodel.h"

ResourceModel::ResourceModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

QModelIndex ResourceModel::index(int row, int column, const QModelIndex &parent) const
{
    return QModelIndex();
}

QModelIndex ResourceModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int ResourceModel::columnCount(const QModelIndex &parent) const
{
    return 2; // title, url
}

int ResourceModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant ResourceModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QVariant ResourceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole)
    switch(section)
    {
    case 0:
        return QVariant(tr("Title"));
    case 1:
        return QVariant(tr("URL"));
    default:
        break;
    }
    return QVariant();
}
