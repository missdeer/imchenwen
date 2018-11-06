#include "player.h"

Player::Player(Player::Type t, const QString n)
    : m_type(t), m_name(n)
{

}

Player::Type Player::type() const
{
    return m_type;
}

void Player::setType(const Player::Type &type)
{
    m_type = type;
}

const QString &Player::name() const
{
    return m_name;
}

void Player::setName(const QString &name)
{
    m_name = name;
}

const QString &Player::arguments() const
{
    return m_arguments;
}

void Player::setArguments(const QString &arguments)
{
    m_arguments = arguments;
}
