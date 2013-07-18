#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractItemModel>

class PlaylistModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit PlaylistModel(QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex	parent(const QModelIndex & index) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const ;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
signals:
    
public slots:
    
};

#endif // PLAYLISTMODEL_H
