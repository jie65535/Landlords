#include "stdafx.h"
#include "Player.h"


Player::Player(playerID_t id):
	m_id(id), m_name(), m_integral(0), m_currRoom(0), isReady(false)
{
}


Player::~Player()
{
}
