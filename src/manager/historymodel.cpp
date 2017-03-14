#include "historymodel.h"

HistoryModel::HistoryModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

QModelIndex HistoryModel::index(int row, int column, const QModelIndex &parent) const
{
    return QModelIndex();
}

QModelIndex HistoryModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int HistoryModel::columnCount(const QModelIndex &parent) const
{
    return 3; // title, original url, target url
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QVariant HistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role == Qt::DisplayRole)
    switch(section)
    {
    case 0:
        return QVariant(tr("Title"));
    case 1:
        return QVariant(tr("Original URL"));
    case 2:
        return QVariant(tr("Target URL"));
    default:
        break;
    }
    return QVariant();
}
