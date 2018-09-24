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
	// 开始  进入playing状态
	void start();
/********** 以下操作只有在进入playing状态后才可使用 ************/
	
	// 得到地主牌
	std::array<card_t, 3> getLandCards();
	
	// 地主的ID  如果还没有地主，返回0
	playerID_t getLandID() { return m_landlord==-1?0:m_PIDList[m_landlord]; }
	
	// 得到目标玩家的手牌
	const std::vector<card_t> &getPlayerCards(playerID_t pid);
	
	// 得到目标玩家的剩余手牌数量
	size_t getPlayerCardsSize(playerID_t pid);
	
	// 得到当前轮到的玩家的ID
	playerID_t getThisTurn() { logFile << "轮到：" << m_turn << std::endl; return m_PIDList.at(m_turn); }
	
	// 叫地主，该函数返回真表示已经处理，返回假表示流局重开
	bool coll(bool isColl);
	// 叫地主了吗
	bool isColledLand() { return m_first != -1; }
	// 抢地主，有人成为地主返回真，否则返回假
	bool grab(bool isGrab);

	// 出牌，参数是出的牌，出牌成功返回真
	// 若上牌时，牌型不合法或者大不过上家 返回假
	// 如果出王炸，下轮任然是该玩家出牌
	// 出牌操作会对参数进行排序
	bool popCards(const PlayingCards & cards);

	// 注意：出牌阶段无法pass，若这么做返回假
	bool pass();

	// 得到最后一轮手牌
	const std::vector<card_t> & getLastRoundCards() { return m_lastRoundCards.getVec(); }

	// 是否出牌阶段（非上牌阶段）
	bool isPopCards() { return m_turn == m_lastPop; }

	// 是否游戏已经结束
	bool isEndGame() { return m_roomStatus == roomStatus_t::Waiting; }
	// 结算分数  返回得分
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


	// 游戏结束，进入等待状态，清理
	void endGame();
	
private:
	roomID_t m_id;
	roomStatus_t m_roomStatus;
	std::vector<playerID_t> m_PIDList;
	std::array<card_t, 54> m_cards;
	std::array<std::vector<card_t>, roomPlayerLimit> m_playerCards;
	PlayingCards m_lastRoundCards;   // 最后一轮牌
/*********************************************************/
	int m_first;                   // 第一个叫地主的玩家
	bool m_Grad[roomPlayerLimit];  // 抢地主资格
/*********************************************************/
	int m_lastPop;                 // 最后一个出牌/抢地主的玩家
	int m_turn;                    // 当前轮到的玩家
	int m_landlord;                // 地主
	int m_X;                       // 房间系数  得分公式 X * m (地主*2)
	uint32_t m_multiple;           // 倍数
};

