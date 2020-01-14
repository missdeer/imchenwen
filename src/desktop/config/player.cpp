#include "player.h"

Player::Player(Player::Type t, const QString &title, const QString &path) : m_type(t), m_title(title), m_path(path) {}

Player::Type Player::type() const
{
    return m_type;
}

void Player::setType(const Player::Type &type)
{
    m_type = type;
}

const QString &Player::path() const
{
    return m_path;
}

void Player::setPath(const QString &path)
{
    m_path = path;
}

const QString &Player::arguments() const
{
    return m_arguments;
}

void Player::setArguments(const QString &arguments)
{
    m_arguments = arguments;
}

const QString &Player::title() const
{
    return m_title;
}

void Player::setTitle(const QString &title)
{
    m_title = title;
}
