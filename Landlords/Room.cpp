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
	//logFile << "���ID:<" << pid << ">��ͼ���뷿��:" << m_id << std::endl;
	if (m_PIDList.size() == 3)
	{
		logFile << m_id << "��������" << std::endl;
		return false;
	}
	m_PIDList.push_back(pid);
	logFile << "���ID:<" << pid << ">�Ѿ����뷿��:" << m_id << std::endl;
	return true;
}

bool Room::playerLeave(playerID_t pid)
{
	//logFile << "���ID:<" << pid << ">��ͼ�뿪����:" << m_id << std::endl;
	if (m_PIDList.empty())
	{
		logFile << "����Ϊ��" << std::endl;
		return false;
	}
	auto it = std::find(m_PIDList.begin(), m_PIDList.end(), pid);
	if (it == m_PIDList.end())
	{
		logFile << "�ڸ÷����Ҳ������ID" << std::endl;
		return false;
	}
	m_PIDList.erase(it);
	logFile << "���ID:<" << pid << ">�Ѿ��뿪����:" << m_id << std::endl;
	return true;
}

void Room::start()
{
	m_roomStatus = roomStatus_t::playing;
	logFile << "����ID<" << m_id << ">�Ѿ���ʼ��Ϸ" << std::endl;
	m_landlord = -1; // ������ʼ��Ϊ��
	m_lastPop = -1;
	m_first = -1;
	m_turn = landU(e);  // �����һ���˿�ʼ
	logFile << "�������" << m_turn << std::endl;
	m_multiple = 1;
	for (int i = 0; i < roomPlayerLimit; ++i)
		m_Grad[i] = true;
	Refresh();       // ϴ��
	distribution();  // ����
}

std::array<card_t, 3> Room::getLandCards()
{
	// ��������ǵ�����
	return std::array<card_t, 3>{m_cards[51], m_cards[52], m_cards[53]};
}

const std::vector<card_t>& Room::getPlayerCards(playerID_t pid)
{
	//logFile << "���ID<" << pid << ">���Եõ��Լ�����" << std::endl;
	return m_playerCards[findPlayer(pid)];
}

size_t Room::getPlayerCardsSize(playerID_t pid)
{
	//logFile << "���ID<" << pid << ">���Եõ��Ƶ�����" << std::endl;
	return m_playerCards[findPlayer(pid)].size();
}

int Room::settle(playerID_t pid)
{
	int score = m_X * m_multiple;
	// �������
	if (pid == getLandID())  // ����ǵ���
	{
		score *= 2;          // ���ַ���
		if (m_turn != m_landlord)  // �����ƵĲ��ǵ���
			score = -score;  // �۷�
	}
	else {                   // ���ǵ���
		if (m_turn == m_landlord)  //�����Ƶ��ǵ���
			score = -score;  // �۷�
	}
	return score;
}

int Room::findPlayer(playerID_t pid)
{
	//logFile << "ID<" << pid << ">���Բ����ڵ�ǰ�����±�" << std::endl;
	int i = 0;
	for (; i < 3; ++i)
		if (m_PIDList[i] == pid)
			break;
	if (i == 3)
	{
		logFile << "�����Ҳ����������" << std::endl;
		throw "����Ҳ��ڵ�ǰ����";
	}
	else
		return i;
}

void Room::setLandlords(int n)
{
	m_lastPop = m_turn = m_landlord = n;
	logFile << n << "��Ϊ����" << std::endl;
	// ��������
	m_playerCards[m_landlord].insert(m_playerCards[m_landlord].end(),
		m_cards.end() - 3, m_cards.end());
	// ��������
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
		logFile << m_turn << " �е���" << std::endl;
		m_lastPop = m_first = m_turn;
		for (turnNext(); !m_Grad[m_turn]; turnNext())
			logFile << m_turn << "û�ʸ�������һ��" << std::endl;
		logFile << m_turn << "���ʸ�" << std::endl;
		if (m_first == m_turn)
		{
			setLandlords(m_turn);
			logFile << "���һ���У��õ�����" << std::endl;
			return true;
		}
		return true;
	}
	m_Grad[m_turn] = false;
	logFile << m_turn << "���е�����ʧȥ�������ʸ�" << std::endl;
	turnNext();
	if (!m_Grad[m_turn]) // ���û�����ʸ�е�����˵��û�˽е���
	{
		logFile << "û�˽е����������ؿ�" << std::endl;
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
		logFile << m_turn << "������" << std::endl;
		m_lastPop = m_turn;
		doubleMultiple();
		for (turnNext(); !m_Grad[m_turn]; turnNext())
			logFile << m_turn << "û�ʸ�������һ��" << std::endl;
		logFile << m_turn << "���ʸ�" << std::endl;
		if (m_lastPop == m_turn) // ���תһȦ������������ˣ�˵������������һ�����ʸ���������
		{
			logFile << m_turn << "��Ϊ����" << std::endl;
			setLandlords(m_lastPop);
			return true;
		}
		m_Grad[m_lastPop] = false;
	}
	else
	{
		int temp = m_turn;
		logFile << m_turn << "����" << std::endl;
		for (turnNext(); !m_Grad[m_turn]; turnNext())
			logFile << m_turn << "û�ʸ�������һ��" << std::endl;
		logFile << m_turn << "���ʸ�" << std::endl;
		
		if (temp == m_turn)  // ��������˶�û�ʸ��������������������������˵���(���� �� ����...)
		{
			setLandlords(m_lastPop);
			return true;
		}

		// �����һ�����ʸ�ľ��ǽе����ģ�ͬʱ��Ҳ�����һ��Ҫ�������ģ�ֱ�ӳ�Ϊ����
		if (m_turn == m_first && m_first == m_lastPop) 
		{
			logFile << m_turn << "��Ϊ����" << std::endl;
			setLandlords(m_first);
			return true;
		}
		m_Grad[temp] = false;
	}
	return false;
}

bool Room::popCards(const PlayingCards & cards)
{
	logFile << "���� ���� " << cards.getType() << std::endl;
	if (cards.getType() == c0)
		return false;
	if (!isSubset(cards.getVec()))
	{
		logFile << "û�����" << std::endl;
		return false;
	}
	// ��ը �������ӶԹ����
	if (cards.getType() == c11)
	{
		logFile << "��ը" << std::endl;
		doubleMultiple();
	}
	// ���ƽ׶�  ֻҪ �������͵� �����е� �����Գ���
	else if (isPopCards())
	{
		if (cards.getType() == c4)  // �����ը������ը�� ��������
		{
			logFile << "ը��" << std::endl;
			doubleMultiple();
		}
		// �����Ķ����Գ�
	}
	// ����������ƽ׶� 
	else if (cards.getType() == c4) // ը��
	{
		if (!(m_lastRoundCards < cards))// û�ϼҴ�
			return false;
	}
	// �ų���ը������ը��������Ͳ�ֱͬ�ӷ��ؼ�
	else if (cards.getType() != m_lastRoundCards.getType())
	{
		return false;
	}
	// �ų������Ͳ�ͬ������������ͬ��һ�����ؼ�
	else if (cards.getSize() != m_lastRoundCards.getSize())
	{
		return false;
	}
	// ������������ͬ��������ͬ�����󲻹��ϼң�һ�����ؼ�
	else if (!(m_lastRoundCards < cards))
	{
		return false;
	}
	// ��������ƿ϶��ǵó���ȥ��
	m_lastPop = m_turn;
	m_lastRoundCards = cards;
	removeSubset(cards.getVec());
	logFile << "���Ƴɹ�" << std::endl;

	if (m_playerCards[m_turn].empty())
	{
		endGame();
		return true;
	}

	if (cards.getType() != c11) // �������ը��Ҳ�����ֵ��¼���
		turnNext();
	return true;
}

bool Room::pass()
{
	logFile << "����" << std::endl;
	// ����Ϊ���ƽ׶� �޷�����
	if (m_turn == m_lastPop)  // ���һ�����Ƶľ��������
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
		logFile << i << "����ҵõ��ˣ�";
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
