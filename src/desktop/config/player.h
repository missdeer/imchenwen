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

    Player(Type t, const QString n);
    Type type() const;
    void setType(const Type &type);

    const QString &name() const;
    void setName(const QString &name);

    const QString &arguments() const;
    void setArguments(const QString &arguments);

private:
    Type m_type;
    QString m_name;
    QString m_arguments;
};

typedef QSharedPointer<Player> PlayerPtr;
typedef QList<PlayerPtr> PlayerList;

#endif // PLAYER_H
