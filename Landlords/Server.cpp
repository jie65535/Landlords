#include "stdafx.h"
#include "Server.h"
#include <ctime>

std::default_random_engine e;
std::uniform_int_distribution<int> landU(0, 2);        // ѡ����

std::uniform_int_distribution<int> cardsU(0, 53);      // ϴ��


Server::Server()
{
	logFile << "Server::Server()" << std::endl;
	std::ifstream fin("data.dat");
	if (!fin.is_open())
	{
		logFile << "data.dat �ļ���ʧ��" << std::endl;
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
		logFile << "data.dat �ļ���ʧ��" << std::endl;
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
	// ���ڸ÷����������ã����ҷ����б�ȷ�����������Բ��ö��������㷨���Ч��
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
	// ���ڸ÷����������ã����ҷ����б�ȷ�����������Բ��ö��������㷨���Ч��
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
	logFile << "���id:" << pid << " ��ͼ��������" << std::endl;
	Player *pplayer = getPlayer(pid);
	if (pplayer->getCurrRoom() != 0)
	{
		logFile << "��������ʧ�ܣ���Ϊ��������ڷ�����" << std::endl;
		return 0;
	}
	roomID_t rid = 0;
	if (m_roomList.empty())
		rid = 1;
	else
		rid = m_roomList.back().getID() + 1;
	m_roomList.push_back(Room(rid, pid));
	pplayer->setCurrRoom(rid);
	logFile << "���id:" << pid << " ��������ɹ�������id:" << rid << std::endl;
	return rid;
}

playerID_t Server::loginPlayer(Player player)
{
	logFile << "���id:" << player.getID() << "����ע��" << std::endl;
	Player * pplayer = getPlayer(player.getID());
	if (pplayer != nullptr)
	{
		logFile << "���и������Ϣ���޷��ظ�ע��" << std::endl;
		return 0;
	}
	m_playerList.push_back(player);
	std::sort(m_playerList.begin(), m_playerList.end());
	return player.getID();
}

bool Server::joinRoom(playerID_t pid, roomID_t rid)
{
	logFile << "���id:" << pid << " ���Լ��뷿�䣬����id:" << rid << std::endl;
	if (rid == 0)
	{
		logFile << "����ID�Ƿ�" << std::endl;
		return false;
	}
	Player *pplayer = getPlayer(pid);
	if (pplayer == nullptr)
	{
		logFile << "û��������" << std::endl;
		return false;
	}
	if (pplayer->getCurrRoom() == rid)
	{
		logFile << "����ҵ�ǰ����Ŀ�귿��" << std::endl;
		return false;
	}
	Room * proom = getRoom(rid);
	if (proom == nullptr)
	{
		logFile << "�Ҳ����÷���" << std::endl;
		return false;
	}
	else {
		// ��������ڷ��䣬�����˳���ǰ����
		if (pplayer->getCurrRoom() != 0)
			leaveRoom(pid);
		if (proom->playerJoin(pid))
		{
			pplayer->setCurrRoom(rid);
			pplayer->isReady = false;
			logFile << "���id:" << pid << "�ɹ����뷿��:" << rid << std::endl;
			return true;
		}
		else {
			return false;
		}
	}
}

bool Server::leaveRoom(playerID_t pid)
{
	logFile << "���id:" << pid << " �����˳���ǰ����" << std::endl;
	Player * pplayer = getPlayer(pid);
	if (pplayer == nullptr)
	{
		logFile << "û��������" << std::endl;
		return false;
	}
	roomID_t rid = pplayer->getCurrRoom();
	if (rid == 0)
	{
		logFile << "������û�м����κη��䣬�޷��˳�" << std::endl;
		return false;
	}
	auto it = std::find(m_roomList.begin(), m_roomList.end(), rid);
	if (it->playerLeave(pid))
	{
		pplayer->setCurrRoom(0);
		pplayer->isReady = false;
		logFile << "���id:" << pid << " �Ѿ��˳���ǰ���䣬����ID:" << rid << std::endl;
		if (it->getPlayerIDList().empty())
		{
			logFile << "��ǰ����Ϊ�գ�ɾ���÷���  id:" << rid << std::endl;
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
	logFile << "���id:" << pid << "����׼��/ȡ��׼��" << std::endl;
	if (pplayer == nullptr || rid == 0)
	{
		logFile << "�޸���һ��߷���Ϊ��" << std::endl;
		return false;
	}

	pplayer->isReady = !pplayer->isReady;
	logFile << "���óɹ���pplayer->isReady:" << pplayer->isReady << std::endl;
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
		// ������δ׼������flagΪfalse
		if (!getPlayer(i)->isReady)
			flag = false;
	}
	// ����ʶλδ���üٱ�ʾ�����˶��Ѿ�׼��
	if (flag)
	{
		logFile << "����id:" << proom->getID() << "  �������Ѿ�׼��" << std::endl;
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
