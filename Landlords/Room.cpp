#include "stdafx.h"
#include "Room.h"


Room::Room(roomID_t rid, playerID_t pid) :
	m_id(rid), m_roomStatus(roomStatus_t::Waiting), m_PIDList(),
	m_turn(0), m_landlord(0), m_X(100), m_lastRoundCards(std::vector<card_t>())
{
	logFile << "Room::Room(" << rid << ", " << pid << ")\n";
	playerJoin(pid);
	int n = 0;
	for (int i = 0; i < 52; ++i)
	{
		if (i % 4 == 0)
			++n;
		m_cards[i] = card_t(n);
	}
	m_cards[52] = card_t::_S;
	m_cards[53] = card_t::_B;
}

Room::~Room()
{
}

bool Room::playerJoin(playerID_t pid)
{
	//logFile << "玩家ID:<" << pid << ">试图加入房间:" << m_id << std::endl;
	if (m_PIDList.size() == 3)
	{
		logFile << m_id << "房间已满" << std::endl;
		return false;
	}
	m_PIDList.push_back(pid);
	logFile << "玩家ID:<" << pid << ">已经加入房间:" << m_id << std::endl;
	return true;
}

bool Room::playerLeave(playerID_t pid)
{
	//logFile << "玩家ID:<" << pid << ">试图离开房间:" << m_id << std::endl;
	if (m_PIDList.empty())
	{
		logFile << "房间为空" << std::endl;
		return false;
	}
	auto it = std::find(m_PIDList.begin(), m_PIDList.end(), pid);
	if (it == m_PIDList.end())
	{
		logFile << "在该房间找不到这个ID" << std::endl;
		return false;
	}
	m_PIDList.erase(it);
	logFile << "玩家ID:<" << pid << ">已经离开房间:" << m_id << std::endl;
	return true;
}

void Room::start()
{
	m_roomStatus = roomStatus_t::playing;
	logFile << "房间ID<" << m_id << ">已经开始游戏" << std::endl;
	m_landlord = -1; // 地主初始化为无
	m_lastPop = -1;
	m_first = -1;
	m_turn = landU(e);  // 随机找一个人开始
	logFile << "随机数：" << m_turn << std::endl;
	m_multiple = 1;
	for (int i = 0; i < roomPlayerLimit; ++i)
		m_Grad[i] = true;
	Refresh();       // 洗牌
	distribution();  // 发牌
}

std::array<card_t, 3> Room::getLandCards()
{
	// 最后三张是地主牌
	return std::array<card_t, 3>{m_cards[51], m_cards[52], m_cards[53]};
}

const std::vector<card_t>& Room::getPlayerCards(playerID_t pid)
{
	//logFile << "玩家ID<" << pid << ">尝试得到自己的牌" << std::endl;
	return m_playerCards[findPlayer(pid)];
}

size_t Room::getPlayerCardsSize(playerID_t pid)
{
	//logFile << "玩家ID<" << pid << ">尝试得到牌的数量" << std::endl;
	return m_playerCards[findPlayer(pid)].size();
}

int Room::settle(playerID_t pid)
{
	int score = m_X * m_multiple;
	// 结算分数
	if (pid == getLandID())  // 如果是地主
	{
		score *= 2;          // 积分翻倍
		if (m_turn != m_landlord)  // 最后出牌的不是地主
			score = -score;  // 扣分
	}
	else {                   // 不是地主
		if (m_turn == m_landlord)  //最后出牌的是地主
			score = -score;  // 扣分
	}
	return score;
}

int Room::findPlayer(playerID_t pid)
{
	//logFile << "ID<" << pid << ">尝试查找在当前房间下标" << std::endl;
	int i = 0;
	for (; i < 3; ++i)
		if (m_PIDList[i] == pid)
			break;
	if (i == 3)
	{
		logFile << "这个玩家不在这个房间" << std::endl;
		throw "该玩家不在当前房间";
	}
	else
		return i;
}

void Room::setLandlords(int n)
{
	m_lastPop = m_turn = m_landlord = n;
	logFile << n << "成为地主" << std::endl;
	// 发地主牌
	m_playerCards[m_landlord].insert(m_playerCards[m_landlord].end(),
		m_cards.end() - 3, m_cards.end());
	// 重新排序
	std::sort(m_playerCards[m_landlord].begin(), m_playerCards[m_landlord].end());
}

bool Room::isSubset(const std::vector<card_t>& sub)
{
	auto & vec = m_playerCards[m_turn];
	if (sub.size() > vec.size())
		return false;
	int arr[16] = { 0 };
	for (auto i : sub)
		arr[i]++;
	for (auto i : vec)
		arr[i]--;
	for (auto i : arr)
		if (i > 0)
			return false;
	return true;
}

bool Room::removeSubset(const std::vector<card_t>& sub)
{
	auto & vec = m_playerCards[m_turn];
	if (sub.size() > vec.size())
		return false;
	int arr[16] = { 0 };
	for (auto i : sub)
		arr[i]++;
	for (auto it = vec.begin(); it != vec.end(); )
		if (arr[*it]-- > 0)
			it = vec.erase(it);
		else
			++it;
	return true;
}

bool Room::coll(bool isColl)
{
	if (m_first != -1)
		return true;
	if (isColl)
	{
		logFile << m_turn << " 叫地主" << std::endl;
		m_lastPop = m_first = m_turn;
		for (turnNext(); !m_Grad[m_turn]; turnNext())
			logFile << m_turn << "没资格抢，下一个" << std::endl;
		logFile << m_turn << "有资格" << std::endl;
		if (m_first == m_turn)
		{
			setLandlords(m_turn);
			logFile << "最后一个叫，得到地主" << std::endl;
			return true;
		}
		return true;
	}
	m_Grad[m_turn] = false;
	logFile << m_turn << "不叫地主，失去抢地主资格" << std::endl;
	turnNext();
	if (!m_Grad[m_turn]) // 如果没人有资格叫地主，说明没人叫地主
	{
		logFile << "没人叫地主，流局重开" << std::endl;
		start();
		return false;
	}
	return true;
}

bool Room::grab(bool isGrab)
{
	if (m_landlord != -1)
		return false;
	if (isGrab)
	{
		logFile << m_turn << "抢地主" << std::endl;
		m_lastPop = m_turn;
		doubleMultiple();
		for (turnNext(); !m_Grad[m_turn]; turnNext())
			logFile << m_turn << "没资格抢，下一个" << std::endl;
		logFile << m_turn << "有资格" << std::endl;
		if (m_lastPop == m_turn) // 如果转一圈出来还是这个人，说明这个人是最后一个有资格抢地主的
		{
			logFile << m_turn << "成为地主" << std::endl;
			setLandlords(m_lastPop);
			return true;
		}
		m_Grad[m_lastPop] = false;
	}
	else
	{
		int temp = m_turn;
		logFile << m_turn << "不抢" << std::endl;
		for (turnNext(); !m_Grad[m_turn]; turnNext())
			logFile << m_turn << "没资格抢，下一个" << std::endl;
		logFile << m_turn << "有资格" << std::endl;
		
		if (temp == m_turn)  // 如果其它人都没资格再抢地主，则给最后抢地主的人地主(不叫 叫 不抢...)
		{
			setLandlords(m_lastPop);
			return true;
		}

		// 如果下一个有资格的就是叫地主的，同时他也是最后一个要当地主的，直接成为地主
		if (m_turn == m_first && m_first == m_lastPop) 
		{
			logFile << m_turn << "成为地主" << std::endl;
			setLandlords(m_first);
			return true;
		}
		m_Grad[temp] = false;
	}
	return false;
}

bool Room::popCards(const PlayingCards & cards)
{
	logFile << "出牌 牌型 " << cards.getType() << std::endl;
	if (cards.getType() == c0)
		return false;
	if (!isSubset(cards.getVec()))
	{
		logFile << "没这个牌" << std::endl;
		return false;
	}
	// 王炸 天王老子对鬼最大
	if (cards.getType() == c11)
	{
		logFile << "王炸" << std::endl;
		doubleMultiple();
	}
	// 出牌阶段  只要 符合牌型的 手里有的 都可以出。
	else if (isPopCards())
	{
		if (cards.getType() == c4)  // 如果是炸弹（空炸） 倍数翻倍
		{
			logFile << "炸弹" << std::endl;
			doubleMultiple();
		}
		// 其它的都可以出
	}
	// 否则就是上牌阶段 
	else if (cards.getType() == c4) // 炸弹
	{
		if (!(m_lastRoundCards < cards))// 没上家大
			return false;
	}
	// 排除了炸弹和王炸后，如果牌型不同直接返回假
	else if (cards.getType() != m_lastRoundCards.getType())
	{
		return false;
	}
	// 排除了牌型不同后，若牌数量不同，一样返回假
	else if (cards.getSize() != m_lastRoundCards.getSize())
	{
		return false;
	}
	// 最后，如果牌型相同，数量相同，但大不过上家，一样返回假
	else if (!(m_lastRoundCards < cards))
	{
		return false;
	}
	// 最后的最后，牌肯定是得出出去的
	m_lastPop = m_turn;
	m_lastRoundCards = cards;
	removeSubset(cards.getVec());
	logFile << "出牌成功" << std::endl;

	if (m_playerCards[m_turn].empty())
	{
		endGame();
		return true;
	}

	if (cards.getType() != c11) // 如果是王炸，也不用轮到下家了
		turnNext();
	return true;
}

bool Room::pass()
{
	logFile << "不出" << std::endl;
	// 该轮为出牌阶段 无法不出
	if (m_turn == m_lastPop)  // 最后一个出牌的就是这个人
		return false;
	else
	{
		turnNext();
		return true;
	}
}

void Room::Refresh()
{
	for (int i = 0; i < 54; ++i)
		std::swap(m_cards[i], m_cards[cardsU(e)]);
}

void Room::distribution()
{
	for (size_t i = 0; i < m_playerCards.size(); ++i)
	{
		m_playerCards[i].clear();
		m_playerCards[i].insert(m_playerCards[i].begin(), 
			m_cards.begin() + (i * 17), m_cards.begin() + ((i + 1) * 17));
		std::sort(m_playerCards[i].begin(), m_playerCards[i].end());
		logFile << i << "号玩家得到了：";
		for (auto n : m_playerCards[i])
			logFile << n << ' ';
		logFile << std::endl;
	}
}

void Room::endGame()
{
	m_roomStatus = roomStatus_t::Waiting;

	for (auto pid : m_PIDList)
	{
		server().playerReady(pid);
	}

}
