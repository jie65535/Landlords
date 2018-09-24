#include "stdafx.h"
#include "Server.h"
#include <ctime>

std::default_random_engine e;
std::uniform_int_distribution<int> landU(0, 2);        // 选地主

std::uniform_int_distribution<int> cardsU(0, 53);      // 洗牌


Server::Server()
{
	logFile << "Server::Server()" << std::endl;
	std::ifstream fin("data.dat");
	if (!fin.is_open())
	{
		logFile << "data.dat 文件打开失败" << std::endl;
		return;
	}
	std::string str;
	playerID_t pid;
	std::string name;
	int integral = 0;
	while (fin.good() && std::getline(fin, str))
	{
		std::istringstream sin(str);
		sin >> pid >> name >> integral;
		m_playerList.emplace_back(pid);
		m_playerList.back().setName(name);
		m_playerList.back().setIntegral(integral);
	}
	fin.close();

	std::random_device rd;
	e.seed(rd());
	srand((unsigned int)time(NULL));
}


Server::~Server()
{
	if (m_playerList.empty())
		return;
	std::ofstream fout("data.dat");
	if (!fout.is_open())
	{
		logFile << "data.dat 文件打开失败" << std::endl;
		return;
	}
	for (auto i : m_playerList)
	{
		fout << i.getID() << '\t' << i.getName() << '\t' << i.getIntegral() << std::endl;
	}
	fout.close();
}

Room * Server::getRoom(roomID_t rid)
{
	if (m_roomList.empty())
		return nullptr;
	/*
	auto it = m_roomList.begin();
	for (; it != m_roomList.end(); ++it)
		if (it->getID() == rid)
			break;
	if (it == m_roomList.end())
		return nullptr;
	else
		return &(*it);
	*/
	// 由于该方法反复调用，并且房间列表确定已排序，所以采用二分搜索算法提高效率
	int left = 0;
	int right = m_roomList.size() - 1;
	while (left <= right)
	{
		int mid = left + ((right - left) >> 1);
		if (m_roomList[mid].getID() == rid)
			return &m_roomList[mid];
		else if (m_roomList[mid].getID() > rid)
			right = mid - 1;
		else
			left = mid + 1;
	}
	return nullptr;
}

Player * Server::getPlayer(playerID_t pid)
{
	if (m_playerList.empty())
		return nullptr;
	/*
	auto it = m_playerList.begin();
	for (; it != m_playerList.end(); ++it)
		if (it->getID() == pid)
			break;
	if (it == m_playerList.end())
		return nullptr;
	else
		return &(*it);
	*/
	// 由于该方法反复调用，并且房间列表确定已排序，所以采用二分搜索算法提高效率
	int left = 0;
	int right = m_playerList.size() - 1;
	while (left <= right)
	{
		int mid = left + ((right - left) >> 1);
		if (m_playerList[mid].getID() == pid)
			return &m_playerList[mid];
		else if (m_playerList[mid].getID() > pid)
			right = mid - 1;
		else
			left = mid + 1;
	}
	return nullptr;
}

roomID_t Server::createRoom(playerID_t pid)
{
	logFile << "玩家id:" << pid << " 试图创建房间" << std::endl;
	Player *pplayer = getPlayer(pid);
	if (pplayer->getCurrRoom() != 0)
	{
		logFile << "创建房间失败，因为该玩家正在房间中" << std::endl;
		return 0;
	}
	roomID_t rid = 0;
	if (m_roomList.empty())
		rid = 1;
	else
		rid = m_roomList.back().getID() + 1;
	m_roomList.push_back(Room(rid, pid));
	pplayer->setCurrRoom(rid);
	logFile << "玩家id:" << pid << " 创建房间成功，房间id:" << rid << std::endl;
	return rid;
}

playerID_t Server::loginPlayer(Player player)
{
	logFile << "玩家id:" << player.getID() << "尝试注册" << std::endl;
	Player * pplayer = getPlayer(player.getID());
	if (pplayer != nullptr)
	{
		logFile << "已有该玩家信息，无法重复注册" << std::endl;
		return 0;
	}
	m_playerList.push_back(player);
	std::sort(m_playerList.begin(), m_playerList.end());
	return player.getID();
}

bool Server::joinRoom(playerID_t pid, roomID_t rid)
{
	logFile << "玩家id:" << pid << " 尝试加入房间，房间id:" << rid << std::endl;
	if (rid == 0)
	{
		logFile << "房间ID非法" << std::endl;
		return false;
	}
	Player *pplayer = getPlayer(pid);
	if (pplayer == nullptr)
	{
		logFile << "没有这个玩家" << std::endl;
		return false;
	}
	if (pplayer->getCurrRoom() == rid)
	{
		logFile << "该玩家当前已在目标房间" << std::endl;
		return false;
	}
	Room * proom = getRoom(rid);
	if (proom == nullptr)
	{
		logFile << "找不到该房间" << std::endl;
		return false;
	}
	else {
		// 若玩家已在房间，则先退出当前房间
		if (pplayer->getCurrRoom() != 0)
			leaveRoom(pid);
		if (proom->playerJoin(pid))
		{
			pplayer->setCurrRoom(rid);
			pplayer->isReady = false;
			logFile << "玩家id:" << pid << "成功加入房间:" << rid << std::endl;
			return true;
		}
		else {
			return false;
		}
	}
}

bool Server::leaveRoom(playerID_t pid)
{
	logFile << "玩家id:" << pid << " 尝试退出当前房间" << std::endl;
	Player * pplayer = getPlayer(pid);
	if (pplayer == nullptr)
	{
		logFile << "没有这个玩家" << std::endl;
		return false;
	}
	roomID_t rid = pplayer->getCurrRoom();
	if (rid == 0)
	{
		logFile << "这个玩家没有加入任何房间，无法退出" << std::endl;
		return false;
	}
	auto it = std::find(m_roomList.begin(), m_roomList.end(), rid);
	if (it->playerLeave(pid))
	{
		pplayer->setCurrRoom(0);
		pplayer->isReady = false;
		logFile << "玩家id:" << pid << " 已经退出当前房间，房间ID:" << rid << std::endl;
		if (it->getPlayerIDList().empty())
		{
			logFile << "当前房间为空，删除该房间  id:" << rid << std::endl;
			m_roomList.erase(it);
		}
		return true;
	}
	else {
		return false;
	}
}

bool Server::playerReady(playerID_t pid)
{
	Player *pplayer = getPlayer(pid);
	roomID_t rid = pplayer->getCurrRoom();
	logFile << "玩家id:" << pid << "尝试准备/取消准备" << std::endl;
	if (pplayer == nullptr || rid == 0)
	{
		logFile << "无该玩家或者房间为空" << std::endl;
		return false;
	}

	pplayer->isReady = !pplayer->isReady;
	logFile << "设置成功，pplayer->isReady:" << pplayer->isReady << std::endl;
	return true;
}

bool Server::startGame(roomID_t rid)
{
	Room *proom = getRoom(rid);
	if (isRoomAllReady(proom))
	{
		proom->start();
		return true;
	}
	else {
		return false;
	}
}

bool Server::isRoomAllReady(Room* proom)
{
	if (proom == nullptr || !proom->full())
		return false;
	bool flag = true;
	for (auto i : proom->getPlayerIDList())
	{
		// 若有人未准备则置flag为false
		if (!getPlayer(i)->isReady)
			flag = false;
	}
	// 若标识位未被置假表示所有人都已经准备
	if (flag)
	{
		logFile << "房间id:" << proom->getID() << "  所有人已经准备" << std::endl;
		return true;
	}
	else {
		return false;
	}
}




Server & server()
{
	static Server s;
	return s;
}
