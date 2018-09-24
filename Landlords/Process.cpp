#include "stdafx.h"
#include "Process.h"
using namespace std;
extern int ac;
string playerInfoToString(Player* pplayer)
{
	ostringstream out;
	if (pplayer == nullptr)
	{
		out << "�Ҳ����������Ϣ" << endl;
	}
	else {
		out << "�ǳ�:" << pplayer->getName() << '\t'
			<< "id:" << pplayer->getID() << '\t'
			<< "����:" << pplayer->getIntegral() << '\t';
		roomID_t rid = pplayer->getCurrRoom();
		if (rid != 0)
		{
			out << "����ID:" << rid;
		}
		else {
			out << "������";
		}
	}
	return out.str();
}

string playerInfoToString(playerID_t pid)
{
	ostringstream out;
	auto pplayer = server().getPlayer(pid);
	out << playerInfoToString(pplayer);
	return out.str();
}

string playerListToString()
{
	ostringstream out;
	for (auto i : server().getPlayerList())
	{
		out << playerInfoToString(&i) << endl;
	}
	return out.str();
}

string roomInfoToString(roomID_t rid)
{
	ostringstream out;
	Room *proom = server().getRoom(rid);
	if (proom == nullptr)
	{
		out << "�Ҳ����÷���";
		return out.str();
	}
	out << "����id:" << proom->getID() << endl;
	auto plist = proom->getPlayerIDList();
	out << "��������:" << plist.size() << endl;
	out << "��ϸ��Ϣ:" << endl;
	for (auto i : plist)
	{
		auto pplayer = server().getPlayer(i);
		if (pplayer == nullptr)
		{
			out << "�Ҳ����������Ϣ" << endl;
		}
		else {
			out << "�ǳ�:" << pplayer->getName()
				<< "\t����:" << pplayer->getIntegral()
				<< (pplayer->isReady ? "\t��׼��" : "\tδ׼��")
				<< endl;
		}
	}
	return out.str();
}

std::string roomListToString(bool isHideFull)
{
	std::ostringstream out;
	auto & vec = server().getRoomList();
	if (vec.empty())
	{
		out << "[��]";
	}
	else {
		bool flag = false;
		for (auto &i : vec)
		{
			if (isHideFull && i.full())
			{
				continue;
			}
			if (flag == false)
			{
				out << "ID\t����\t״̬" << endl;
				flag = true;
			}
			out << i.getID() << '\t' << i.getSize() << '/' << roomPlayerLimit << '\t';
			if (i.getStatus() == roomStatus_t::Waiting)
				out << "�ȴ���";
			else if (i.getStatus() == roomStatus_t::playing)
				out << "��Ϸ��";
			out << endl;
		}
		if (flag == false)
			out << "[��]";
	}
	return out.str();
}

inline playerID_t QQToPID(uint64_t fromQQ)
{
	return playerID_t(fromQQ);
}
inline uint64_t PIDToQQ(playerID_t pid)
{
	return uint64_t(pid);
}

// �㲥��Ϣ��ָ�����ID�ķ��䣨����Ϣ�����͸�ָ�����֮�������ˣ�
void broadcast(playerID_t pid, const string& msg)
{
	//logFile << "���Թ㲥���÷���ָ��ID��������ң�ָ��ID��" << pid << std::endl;
	Player * pplayer = server().getPlayer(pid);
	if (pplayer == nullptr)
	{
		logFile << "û������" << std::endl;
		return;
	}

	Room *proom = server().getRoom(pplayer->getCurrRoom());
	if (proom == nullptr)
	{
		logFile << "û�������" << std::endl;
		return;
	}
	for (const auto i : proom->getPlayerIDList())
	{
		if (pid != i)
		{
	//		logFile << "���ڷ��͸�pid:" << i;
			CQ_sendPrivateMsg(ac, PIDToQQ(i), msg.c_str());
	//		logFile << " --- ���" << std::endl;
		}
	}
}
// �㲥��Ϣ��ָ������ID�ķ��䣨����Ϣ�����͸��÷��������ˣ�
void broadcast(roomID_t rid, const string& msg)
{
	//logFile << "���Թ㲥���÷���������ң�����ID��" << rid << std::endl;
	
	Room *proom = server().getRoom(rid);
	if (proom == nullptr)
	{
		logFile << "û�������" << std::endl;
		return;
	}
	for (const auto i : proom->getPlayerIDList())
	{
	//	logFile << "���ڷ��͸�pid:" << i;
		CQ_sendPrivateMsg(ac, PIDToQQ(i), msg.c_str());
	//	logFile << " --- ���" << std::endl;
	}
}

void instruction(uint64_t fromQQ, string msg)
{
	ostringstream out;
	playerID_t pid = QQToPID(fromQQ);
	Player *pplayer = server().getPlayer(pid);
	
	
	if (msg == "׼��")
	{
		if (server().playerReady(pid))
		{
			if (pplayer->isReady)
			{
				out << "��׼��";
				roomID_t rid = pplayer->getCurrRoom();
				broadcast(pid, "�����׼��");
				
				// �����ʼ��Ϸ�˾ͷ��Ƹ���ң�����ֱ�ӷ���
				if (server().startGame(rid))
				{
					broadcast(rid, "��Ϸ�Ѿ���ʼ��");
					broadcast(rid, roomCardsToString(rid));
					Room *proom = server().getRoom(rid);
					// ��������ҿ���
					for (auto pid : proom->getPlayerIDList())
						CQ_sendPrivateMsg(ac, PIDToQQ(pid), CardsToString(proom->getPlayerCards(pid)).c_str());
					NotifyNextPlayer(rid, "�Ƿ�е��� [�е���] [����]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]�е���");
					return;
				}
			}
			else {
				out << "��ȡ��";
				broadcast(pid, "�����ȡ��׼��");
			}
			broadcast(pplayer->getCurrRoom(), string("��ǰ������Ϣ��\n") + roomInfoToString(pplayer->getCurrRoom()));
		}
		else {
			out << "����ʧ��";
		}
	}
	else if (msg == "��������")
	{
		roomID_t rid = server().createRoom(pid);
		if (rid != 0)
			out << "�����ɹ�";
		else
			out << "����ʧ��";
	}
	else if (msg.substr(0, 4) == "����") {
		std::istringstream tempin(msg.substr(4));
		roomID_t rid = 0;
		tempin >> rid;
		if (tempin.bad())
		{
			out << "�����Ƿ��������� ����+ID" << endl;
			out << "�������� 1";
		}
		else if (server().joinRoom(pid, rid))
		{
			out << "���뷿��ɹ�";
			broadcast(pid, "������Ҽ��룡");
			broadcast(rid, string("��ǰ������Ϣ��\n") + roomInfoToString(rid));
		}
		else {
			out << "���뷿��ʧ��";
		}
	}
	else if (msg == "�˳�����")
	{
		roomID_t rid = pplayer->getCurrRoom();
		if (server().leaveRoom(pid))
		{
			out << "�˳�����ɹ�";
			if (server().getRoom(rid))
			{
				broadcast(rid, string("������˳���\n��ǰ������Ϣ��\n") + roomInfoToString(rid));
			}
		}
		else {
			out << "�˳�ʧ��";
		}
	}
	else if (msg == "�鿴����")
	{
		roomID_t rid = pplayer->getCurrRoom();
		if (rid == 0)
			out << "�㵱ǰ���ڴ������޷��鿴��ǰ����";
		else
			out << roomInfoToString(rid);
	}
	else if (msg.substr(0, 8) == "�����б�")
	{
		if (msg[8] == '1')
		{
			out << roomListToString(true);
		}
		else {
			out << roomListToString(false);
			out << "\n�� *�����б�1 �����������˷���";
		}
	}
	else if (msg == "����б�")
	{
		out << playerListToString();
	}
	else if (pplayer->getCurrRoom() != 0)
	{
		msg = "���[" + pplayer->getName() + "]˵��" + msg;
		broadcast(pplayer->getCurrRoom(), msg.c_str());
	}
	CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
}

std::string roomCardsToString(roomID_t rid)
{
	ostringstream out;
	Room *proom = server().getRoom(rid);
	if (proom == nullptr)
		out << "δ���뷿��";
	else if (proom->getStatus() != roomStatus_t::playing)
		out << "��ǰ����δ��ʼ��Ϸ";
	else
	{
		for (auto pid : proom->getPlayerIDList())
		{
			Player *pplayer = server().getPlayer(pid);
			if (proom->getLandID() != 0)
			{
				if (proom->getLandID() == pid)
					out << "����";
				else
					out << "ũ��";
			}
			out << '[' << pplayer->getName() << "]\tʣ������������" << proom->getPlayerCardsSize(pid) << endl;
		}
	}
	return out.str();
}

std::string CardToString(card_t card)
{
	std::string str;
	switch (card)
	{
		case _3:str = " 3";break;
		case _4:str = " 4";break;
		case _5:str = " 5";break;
		case _6:str = " 6";break;
		case _7:str = " 7";break;
		case _8:str = " 8";break;
		case _9:str = " 9";break;
		case _10:str = "10";break;
		case _J:str = " J";break;
		case _Q:str = " Q";break;
		case _K:str = " K";break;
		case _A:str = " A";break;
		case _2:str = " 2";break;
		case _S:str = "С";break;
		case _B:str = "��"; break;
	}
	str = "[" + str + "]";
	return str;
}

std::string CardsToString(const vector<card_t> &cards)
{
	ostringstream out;
	for (auto i : cards)
	{
		out <<  CardToString(i);
	}
	return out.str();
}

std::string CardTtypeToString(CardType ct)
{
	std::string str;
	switch (ct)
	{
	case c1:			str = "����"; break;
	case c2:			str = "����"; break;
	case c3:			str = "����"; break;
	case c4:			str = "ը��"; break;
	case c11:			str = "��ը"; break;
	case c31:			str = "����һ"; break;
	case c32:			str = "������"; break;
	case c411:			str = "�Ĵ���"; break;
	case c422:			str = "�Ĵ�����"; break;
	case c123:			str = "˳��"; break;
	case c1122:			str = "����"; break;
	case c111222:		str = "�ɻ�"; break;
	case c11122234:		str = "�ɻ������"; break;
	case c1112223344:	str = "�ɻ������"; break;
	case c0:			str = "���ܳ���"; break;
	}
	return str;
}

vector< card_t > StringToCards(const string src) {
	vector< card_t > cards;
	card_t temp;
	for (size_t i = 0; i < src.size(); ++i) {
		if (src[i] == '1' && src[i + 1] == '0') {
			i++;
			temp = _10;
		}
		else if (src.substr(i, i + 4) == "����") {
			i += 3;
			temp = _B;
		}
		else if (src.substr(i, i + 2) == "��"){
			i += 1;
			temp = _B;
		}
		else if (src.substr(i, i + 4) == "С��") {
			i += 3;
			temp = _S;
		}
		else if (src.substr(i, i + 2) == "С") {
			i += 1;
			temp = _S;
		}
		else if (src.substr(i, i + 4) == "��ը") {
			cards.push_back(_B);
			cards.push_back(_S);
			break;
		}
		else {
			switch (src[i]) {
			case 'a':
			case 'A':
				temp = _A;
				break;
			case '2':
				temp = _2;
				break;
			case '3':
				temp = _3;
				break;
			case '4':
				temp = _4;
				break;
			case '5':
				temp = _5;
				break;
			case '6':
				temp = _6;
				break;
			case '7':
				temp = _7;
				break;
			case '8':
				temp = _8;
				break;
			case '9':
				temp = _9;
				break;
			case 'j':
			case 'J':
				temp = _J;
				break;
			case 'q':
			case 'Q':
				temp = _Q;
				break;
			case 'k':
			case 'K':
				temp = _K;
				break;
			default:
				continue;
			}
		}
		cards.push_back(temp);
	}
	return cards;
}

void becomeLandlord(Room * proom)
{
	ostringstream out;
	Player * pLand = server().getPlayer(proom->getLandID());
	broadcast(pLand->getID(), "���[" + pLand->getName() + "]��Ϊ�˵�����");
	out << "���Ϊ�˵��������ƣ�\n";
	out << CardsToString(proom->getPlayerCards(pLand->getID()));
	string temp;
	for (auto i : proom->getLandCards())
		temp += CardToString(i);
	broadcast(proom->getID(), "�����ƣ�" + temp);
	CQ_sendPrivateMsg(ac, PIDToQQ(pLand->getID()), out.str().c_str());
	NotifyNextPlayer(proom->getID(), "�ֵ������", "�ȴ���������");
}

// ֪ͨ�����������
// ���� msg      �Ƿ��͸������ֵ�����ҵ���Ϣ
// ���� otherMsg �Ƿ��͸�������ҵ���Ϣ
void NotifyNextPlayer(roomID_t rid, std::string msg, std::string otherMsg)
{
	Room * proom = server().getRoom(rid);
	for (auto pid : proom->getPlayerIDList())
	{
		if (proom->getThisTurn() == pid)
		{
			CQ_sendPrivateMsg(ac, PIDToQQ(pid), msg.c_str());
			CQ_sendPrivateMsg(ac, PIDToQQ(pid), CardsToString(proom->getPlayerCards(pid)).c_str());
		}
		else
		{
			CQ_sendPrivateMsg(ac, PIDToQQ(pid), otherMsg.c_str());
		}
	}
}


// ��Ϸָ��
void gameCommand(uint64_t fromQQ, string msg)
{
	ostringstream out;
	playerID_t pid = QQToPID(fromQQ);
	Player *pplayer = server().getPlayer(pid);
	roomID_t rid = pplayer->getCurrRoom();
	Room * proom = server().getRoom(rid);
do {/********************  do while �� ��������  ************************/
	if (pplayer == nullptr ||
		proom == nullptr ||
		proom->getStatus() != roomStatus_t::playing)
	{
		out << "��ǰ״̬�޷�ʹ����Ϸָ���������Ϸ״̬�²ſ���ʹ��";
		break;
	}
	if (msg == "����")
	{
		out << roomCardsToString(rid);
		out << CardsToString(proom->getPlayerCards(pid));
		break;
	}
	if (msg == "��" || msg == "����" || msg == "�е���")
	{
		if (proom->getLandID() != 0)
		{
			out << "�Ѿ������������Ľ׶Σ�";
			break;
		}
		if (proom->getThisTurn() != pid)
		{
			out << "��û�ֵ���Ŷ��";
			break;
		}
		if (proom->isColledLand())
		{
			out << "�������������Ľ׶Σ�";
			break;
		}
		if (msg == "��" || msg == "�е���")
		{
			proom->coll(true);
			broadcast(pid, "���["+pplayer->getName()+"]�е���");
			
			if (proom->getLandID() == pid)
			{
				becomeLandlord(proom);
			}
			else
			{
				NotifyNextPlayer(rid, "�Ƿ������� [������] [����]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]������");
			}
		}
		else
		{
			if (!proom->coll(false)) // ������ؼٱ�ʾû�˽е������Զ��ؿ���Ϸ
			{
				broadcast(rid, "�����˶�û�е����������ؿ�");
				// ��������ҿ���
				for (auto pid : proom->getPlayerIDList())
					CQ_sendPrivateMsg(ac, PIDToQQ(pid), CardsToString(proom->getPlayerCards(pid)).c_str());
			}
			else
			{
				out << "�㲻�е���";
				broadcast(pid, "���[" + pplayer->getName() + "]���е���");
			}
			NotifyNextPlayer(rid, "�Ƿ�е��� [�е���] [����]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]�е���");
		}
	}
	else if (msg == "��" || msg == "����" 
		|| msg == "����" || msg == "������")
	{
		if (proom->getLandID() != 0)
		{
			out << "�Ѿ������������Ľ׶Σ�";
			break;
		}
		if (proom->getThisTurn() != pid)
		{
			out << "��û�ֵ���Ŷ��";
			break;
		}
		if (!proom->isColledLand())
		{
			out << "�����ǽе����Ľ׶Σ�";
			break;
		}
		if (msg == "��" || msg == "����" ||  msg == "������")
		{
			broadcast(pid, "���[" + pplayer->getName() + "]������");
			if (proom->grab(true))
			{
				becomeLandlord(proom);
			}
			else {
				NotifyNextPlayer(rid, "�Ƿ������� [������] [����]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]������");
			}
		}
		else
		{
			broadcast(pid, "���[" + pplayer->getName() + "]��������");
			if (proom->grab(false))
			{
				becomeLandlord(proom);
			}
			else {
				NotifyNextPlayer(rid, "�Ƿ������� [������] [����]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]������");
			}
		}
	}
	else if (msg.substr(0, 2) == "��")
	{
		if (proom->getLandID() == 0)
		{
			out << "���ڻ����������Ľ׶Σ�";
			break;
		}
		if (proom->getThisTurn() != pid)
		{
			out << "��û�ֵ���Ŷ��";
			break;
		}
		vector<card_t> vec = StringToCards(msg.substr(2));
		PlayingCards pc(vec);
		logFile << CardsToString(vec) << std::endl;
		if (pc.getType() == c0)
		{
			out << "������Ʋ����Ϲ���";
		}
		if (proom->popCards(pc))
		{
			ostringstream outTemp;
			outTemp << "���[";
			outTemp << pplayer->getName();
			outTemp << "]�����[";
			outTemp << CardTtypeToString(pc.getType());
			outTemp << "]\n";
			outTemp << CardsToString(vec);
			broadcast(rid, outTemp.str());
			outTemp.str("");
			if (proom->isEndGame())
			{
				outTemp << "��Ϸ������" << std::endl;;
				if (pplayer->getID() == proom->getLandID())
					outTemp << "����ʤ����" << std::endl;
				else
					outTemp << "ũ��ʤ����" << std::endl;
				for (auto pid : proom->getPlayerIDList())
				{
					int score = proom->settle(pid);
					Player *pPlayerTemp = server().getPlayer(pid);
					pPlayerTemp->score(score);
					outTemp << "���[" << pPlayerTemp->getName() << "] �÷�:" << score << std::endl;
				}
				outTemp << "���� �鿴���� ���鿴��ǰ�������\n���� �鿴 ���鿴��ǰ������Ϣ";
				broadcast(rid, outTemp.str());
				return;
			}

			if (proom->isPopCards())
				NotifyNextPlayer(rid, "����� [�� xxx]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]����");
			else
				NotifyNextPlayer(rid, "����� [�� xxx] [����]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]����");
		}
		else
		{
			out << "�������û���ϼҴ��ϼҳ����ƣ�\n";
			out << CardsToString(proom->getLastRoundCards());
		}
	}
	else if (msg == "����" || msg == "��Ҫ" || msg == "Ҫ����")
	{
		if (proom->getLandID() == 0)
		{
			out << "���ڻ����������Ľ׶Σ�";
			break;
		}
		if (proom->getThisTurn() != pid)
		{
			out << "��û�ֵ���Ŷ��";
			break;
		}
		if (proom->pass())
		{
			broadcast(rid, "���[" + pplayer->getName() + "]����");
			if (proom->isPopCards())
				NotifyNextPlayer(rid, "����� [�� xxx]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]����");
			else
				NotifyNextPlayer(rid, "����� [�� xxx] [����]", "�ȴ�[" + server().getPlayer(proom->getThisTurn())->getName() + "]����");
		}
		else
		{
			out << "�����ǳ��ƽ׶Σ��޷�pass������ƣ�";
		}
	}
	
	else if (msg == "ǿ���ؿ�")
	{
		logFile << "ǿ���ؿ���Ϸ";
		broadcast(rid, "ǿ���ؿ���Ϸ������ ���� �鿴����");
		proom->start();
	}
	else
	{
		msg = "���[" + pplayer->getName() + "]˵��" + msg;
		broadcast(rid, msg.c_str());
		break;
	}
}while(0);/**************  do while �� ��������  ****************/
	CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
}

void Receive(uint64_t fromQQ, string msg)
{
	ostringstream out;
	playerID_t pid = QQToPID(fromQQ);
	Player *pplayer = server().getPlayer(pid);
	if (msg.substr(0, 4) == "ע��" && pplayer == nullptr)
	{
		string name;
		istringstream in(msg.substr(4));
		in >> name;
		Player temp(pid);
		temp.setName(name.substr(0, 12));
		if (server().loginPlayer(temp))
		{
			out << "ע��ɹ����ǳƣ�" << name << std::endl;
			out << "���� �˵� ���õ�ָ���б�";
		}
		else {
			out << "ע��ʧ�ܣ�";
		}
		CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
		return;
	}
	if (pplayer == nullptr)
	{
		out << "��ǰQQδע�ᣬ��ظ� ע�� �ǳ� �����ע�ᣡ" << std::endl;
		out << "���ӣ�ע�� ��������" << std::endl;
		out << "��ע�⣬�ǳ��6�����ֻ���12����ĸ,ע����������,�����в������пո�";
		CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
		return;
	}
	Room *proom = server().getRoom(pplayer->getCurrRoom());
	if (msg == "�˵�")
	{
		out <<
			"+--------ָ���б�--------+\n"
			"| �˵� -- ���ָ���б�   |\n"
			"| ע�� -- ע�� �ǳ�      |\n"
			"| ���� -- ���� ���ǳ�    |\n"
			"| �鿴 -- �鿴��ǰ��Ϣ   |\n"
			"| �������� -- β�������� |\n"
			"| [��Ϣ] -- �ڷ��������� |\n"
			"|----    ����ָ��    ----|\n"
			"|׼�� -- ����׼������ȡ��|\n"
			"|�������� -- ����һ������|\n"
			"|����[id] -- ����ָ������|\n"
			"|�˳����� -- �˳���ǰ����|\n"
			"|�鿴���� -- �鿴��ǰ����|\n"
			"|�����б� -- �鿴���з���|\n"
			"|����б� -- �鿴�������|\n"
			"|*ע�����ָ����Ϸ�н���*|\n"
			"|----    ��Ϸָ��    ----|\n"
			"|���� -- �鿴��ǰ�ƾ����|\n"
			"|�е���/����  ������/����|\n"
			"|����/����    �� xxxx    |\n"
			"|*ע����Ϸָ������н���*|\n"
			"+--------ָ���б�--------+";
	}
	else if (msg == "�鿴")
	{
		out << playerInfoToString(pid);
	}
	else if (msg.substr(0, 4) == "����")
	{
		string name;
		istringstream in(msg.substr(4));
		in >> name;
		pplayer->setName(name.substr(0, 12));
		out << "�޸���ɣ���ǰ�ǳ�:" << name;
	}
	else if (msg == "��������")
	{
		out << "���������ļ��ж��������ݵĹ��ܣ������ڴ�";
	}
	else if (proom == nullptr || proom->getStatus() != roomStatus_t::playing)
		instruction(fromQQ, msg);
	else if (proom->getStatus() == roomStatus_t::playing)
		gameCommand(fromQQ, msg);
	else
		out << "���� �˵� ���õ�ָ���б�";
	CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
}