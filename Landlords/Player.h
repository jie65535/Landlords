#pragma once

#include "define.h"
// #include <string>

class Player
{
public:
	Player() = delete;
	Player(playerID_t id);
	~Player();

	playerID_t getID() { return m_id; }
	int getIntegral() { return m_integral; }
	roomID_t getCurrRoom() { return m_currRoom; }
	void setCurrRoom(roomID_t rid) { m_currRoom = rid; }
	std::string getName() { return m_name; }
	void setName(std::string newName) { m_name = newName; }
	void setIntegral(int integral) { m_integral = integral; }

	// �÷� ����Ϊ�ӷ� ����Ϊ�۷�
	void score(int integral) { m_integral += integral; }

	bool operator==(Player &rhs) { return m_id == rhs.getID(); }
	bool operator==(playerID_t pid) { return m_id == pid; }
	bool operator<(Player &rhs) { return m_id < rhs.getID(); }
	bool operator<(playerID_t pid) { return m_id < pid; }
public:
	bool isReady; // ׼��
private:
	playerID_t m_id;    // ΨһID
	std::string m_name; // �ǳ�
	int m_integral;   // ����
	roomID_t m_currRoom; // ��ǰ���ڷ���(0Ϊ��)
};

