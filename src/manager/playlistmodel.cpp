#include "playlistmodel.h"

PlaylistModel::PlaylistModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

QModelIndex PlaylistModel::index(int row, int column, const QModelIndex &parent) const
{
    return QModelIndex();
}

QModelIndex PlaylistModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int PlaylistModel::columnCount(const QModelIndex &parent) const
{
    return 3; // title, original url, play url
}

int PlaylistModel::rowCount(const QModelIndex &parent) const
{
    return 0;
}

QVariant PlaylistModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QVariant PlaylistModel::headerData(int section, Qt::Orientation orientation, int role) const
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
