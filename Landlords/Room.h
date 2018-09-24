#pragma once

#include "define.h"
#include "Server.h"
#include "PlayingCards.h"
#include <vector>
#include <array>

enum roomStatus_t :char {
	Waiting,
	playing
};

const int roomPlayerLimit = 3;

class Room
{
public:
	Room() = delete;
	Room(roomID_t rid, playerID_t pid);
	~Room();

	roomID_t getID() const { return m_id; };
	roomStatus_t getStatus() const { return m_roomStatus; }
	const std::vector<playerID_t>& getPlayerIDList() const { return m_PIDList; }
	size_t getSize() const { return m_PIDList.size(); }
	// void setStatus(roomStatus_t status) { roomStatus = status; }
	bool full() const { return m_PIDList.size() >= roomPlayerLimit; }
public:
	bool playerJoin(playerID_t pid);
	bool playerLeave(playerID_t pid);

public:
	// ��ʼ  ����playing״̬
	void start();
/********** ���²���ֻ���ڽ���playing״̬��ſ�ʹ�� ************/
	
	// �õ�������
	std::array<card_t, 3> getLandCards();
	
	// ������ID  �����û�е���������0
	playerID_t getLandID() { return m_landlord==-1?0:m_PIDList[m_landlord]; }
	
	// �õ�Ŀ����ҵ�����
	const std::vector<card_t> &getPlayerCards(playerID_t pid);
	
	// �õ�Ŀ����ҵ�ʣ����������
	size_t getPlayerCardsSize(playerID_t pid);
	
	// �õ���ǰ�ֵ�����ҵ�ID
	playerID_t getThisTurn() { logFile << "�ֵ���" << m_turn << std::endl; return m_PIDList.at(m_turn); }
	
	// �е������ú����������ʾ�Ѿ��������ؼٱ�ʾ�����ؿ�
	bool coll(bool isColl);
	// �е�������
	bool isColledLand() { return m_first != -1; }
	// �����������˳�Ϊ���������棬���򷵻ؼ�
	bool grab(bool isGrab);

	// ���ƣ������ǳ����ƣ����Ƴɹ�������
	// ������ʱ�����Ͳ��Ϸ����ߴ󲻹��ϼ� ���ؼ�
	// �������ը��������Ȼ�Ǹ���ҳ���
	// ���Ʋ�����Բ�����������
	bool popCards(const PlayingCards & cards);

	// ע�⣺���ƽ׶��޷�pass������ô�����ؼ�
	bool pass();

	// �õ����һ������
	const std::vector<card_t> & getLastRoundCards() { return m_lastRoundCards.getVec(); }

	// �Ƿ���ƽ׶Σ������ƽ׶Σ�
	bool isPopCards() { return m_turn == m_lastPop; }

	// �Ƿ���Ϸ�Ѿ�����
	bool isEndGame() { return m_roomStatus == roomStatus_t::Waiting; }
	// �������  ���ص÷�
	int settle(playerID_t pid);


public:
	bool operator==(roomID_t rid) { return m_id == rid; }
private:
	int findPlayer(playerID_t pid);
	void doubleMultiple() { m_multiple *= 2; }
	void turnNext() { ++m_turn %= m_PIDList.size(); }
	void setLandlords(int n);
	bool isSubset(const std::vector<card_t> &sub);
	bool removeSubset(const std::vector<card_t> &sub);

	void Refresh();
	void distribution();


	// ��Ϸ����������ȴ�״̬������
	void endGame();
	
private:
	roomID_t m_id;
	roomStatus_t m_roomStatus;
	std::vector<playerID_t> m_PIDList;
	std::array<card_t, 54> m_cards;
	std::array<std::vector<card_t>, roomPlayerLimit> m_playerCards;
	PlayingCards m_lastRoundCards;   // ���һ����
/*********************************************************/
	int m_first;                   // ��һ���е��������
	bool m_Grad[roomPlayerLimit];  // �������ʸ�
/*********************************************************/
	int m_lastPop;                 // ���һ������/�����������
	int m_turn;                    // ��ǰ�ֵ������
	int m_landlord;                // ����
	int m_X;                       // ����ϵ��  �÷ֹ�ʽ X * m (����*2)
	uint32_t m_multiple;           // ����
};

