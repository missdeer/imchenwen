#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QList>
#include <QSharedPointer>

class Player
{
public:
    enum Type {
        PT_BUILTIN,
        PT_EXTERNAL,
        PT_DLNA,
    };

    Player(Type t, const QString &title, const QString &path);
    Type type() const;
    void setType(const Type &type);

    const QString &path() const;
    void           setPath(const QString &path);

    const QString &arguments() const;
    void setArguments(const QString &arguments);

    const QString &title() const;
    void           setTitle(const QString &title);

private:
    Type    m_type;
    QString m_title;
    QString m_path;
    QString m_arguments;
};

typedef QSharedPointer<Player> PlayerPtr;
typedef QList<PlayerPtr> PlayerList;

#endif // PLAYER_H
