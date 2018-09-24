#include "stdafx.h"
#include "Process.h"
using namespace std;
extern int ac;
string playerInfoToString(Player* pplayer)
{
	ostringstream out;
	if (pplayer == nullptr)
	{
		out << "找不到该玩家信息" << endl;
	}
	else {
		out << "昵称:" << pplayer->getName() << '\t'
			<< "id:" << pplayer->getID() << '\t'
			<< "积分:" << pplayer->getIntegral() << '\t';
		roomID_t rid = pplayer->getCurrRoom();
		if (rid != 0)
		{
			out << "房间ID:" << rid;
		}
		else {
			out << "大厅中";
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
		out << "找不到该房间";
		return out.str();
	}
	out << "房间id:" << proom->getID() << endl;
	auto plist = proom->getPlayerIDList();
	out << "房间人数:" << plist.size() << endl;
	out << "详细信息:" << endl;
	for (auto i : plist)
	{
		auto pplayer = server().getPlayer(i);
		if (pplayer == nullptr)
		{
			out << "找不到该玩家信息" << endl;
		}
		else {
			out << "昵称:" << pplayer->getName()
				<< "\t积分:" << pplayer->getIntegral()
				<< (pplayer->isReady ? "\t已准备" : "\t未准备")
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
		out << "[空]";
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
				out << "ID\t人数\t状态" << endl;
				flag = true;
			}
			out << i.getID() << '\t' << i.getSize() << '/' << roomPlayerLimit << '\t';
			if (i.getStatus() == roomStatus_t::Waiting)
				out << "等待中";
			else if (i.getStatus() == roomStatus_t::playing)
				out << "游戏中";
			out << endl;
		}
		if (flag == false)
			out << "[空]";
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

// 广播消息给指定玩家ID的房间（该消息将发送给指定玩家之外所有人）
void broadcast(playerID_t pid, const string& msg)
{
	//logFile << "尝试广播给该房间指定ID外所有玩家，指定ID：" << pid << std::endl;
	Player * pplayer = server().getPlayer(pid);
	if (pplayer == nullptr)
	{
		logFile << "没这个玩家" << std::endl;
		return;
	}

	Room *proom = server().getRoom(pplayer->getCurrRoom());
	if (proom == nullptr)
	{
		logFile << "没这个房间" << std::endl;
		return;
	}
	for (const auto i : proom->getPlayerIDList())
	{
		if (pid != i)
		{
	//		logFile << "正在发送给pid:" << i;
			CQ_sendPrivateMsg(ac, PIDToQQ(i), msg.c_str());
	//		logFile << " --- 完毕" << std::endl;
		}
	}
}
// 广播消息给指定房间ID的房间（该消息将发送给该房间所有人）
void broadcast(roomID_t rid, const string& msg)
{
	//logFile << "尝试广播给该房间所有玩家，房间ID：" << rid << std::endl;
	
	Room *proom = server().getRoom(rid);
	if (proom == nullptr)
	{
		logFile << "没这个房间" << std::endl;
		return;
	}
	for (const auto i : proom->getPlayerIDList())
	{
	//	logFile << "正在发送给pid:" << i;
		CQ_sendPrivateMsg(ac, PIDToQQ(i), msg.c_str());
	//	logFile << " --- 完毕" << std::endl;
	}
}

void instruction(uint64_t fromQQ, string msg)
{
	ostringstream out;
	playerID_t pid = QQToPID(fromQQ);
	Player *pplayer = server().getPlayer(pid);
	
	
	if (msg == "准备")
	{
		if (server().playerReady(pid))
		{
			if (pplayer->isReady)
			{
				out << "已准备";
				roomID_t rid = pplayer->getCurrRoom();
				broadcast(pid, "有玩家准备");
				
				// 如果开始游戏了就发牌给玩家，并且直接返回
				if (server().startGame(rid))
				{
					broadcast(rid, "游戏已经开始！");
					broadcast(rid, roomCardsToString(rid));
					Room *proom = server().getRoom(rid);
					// 给所有玩家看牌
					for (auto pid : proom->getPlayerIDList())
						CQ_sendPrivateMsg(ac, PIDToQQ(pid), CardsToString(proom->getPlayerCards(pid)).c_str());
					NotifyNextPlayer(rid, "是否叫地主 [叫地主] [不叫]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]叫地主");
					return;
				}
			}
			else {
				out << "已取消";
				broadcast(pid, "有玩家取消准备");
			}
			broadcast(pplayer->getCurrRoom(), string("当前房间信息：\n") + roomInfoToString(pplayer->getCurrRoom()));
		}
		else {
			out << "操作失败";
		}
	}
	else if (msg == "创建房间")
	{
		roomID_t rid = server().createRoom(pid);
		if (rid != 0)
			out << "创建成功";
		else
			out << "创建失败";
	}
	else if (msg.substr(0, 4) == "加入") {
		std::istringstream tempin(msg.substr(4));
		roomID_t rid = 0;
		tempin >> rid;
		if (tempin.bad())
		{
			out << "参数非法，请输入 加入+ID" << endl;
			out << "例：加入 1";
		}
		else if (server().joinRoom(pid, rid))
		{
			out << "加入房间成功";
			broadcast(pid, "有新玩家加入！");
			broadcast(rid, string("当前房间信息：\n") + roomInfoToString(rid));
		}
		else {
			out << "加入房间失败";
		}
	}
	else if (msg == "退出房间")
	{
		roomID_t rid = pplayer->getCurrRoom();
		if (server().leaveRoom(pid))
		{
			out << "退出房间成功";
			if (server().getRoom(rid))
			{
				broadcast(rid, string("有玩家退出！\n当前房间信息：\n") + roomInfoToString(rid));
			}
		}
		else {
			out << "退出失败";
		}
	}
	else if (msg == "查看房间")
	{
		roomID_t rid = pplayer->getCurrRoom();
		if (rid == 0)
			out << "你当前正在大厅，无法查看当前房间";
		else
			out << roomInfoToString(rid);
	}
	else if (msg.substr(0, 8) == "房间列表")
	{
		if (msg[8] == '1')
		{
			out << roomListToString(true);
		}
		else {
			out << roomListToString(false);
			out << "\n发 *房间列表1 可以隐藏满人房间";
		}
	}
	else if (msg == "玩家列表")
	{
		out << playerListToString();
	}
	else if (pplayer->getCurrRoom() != 0)
	{
		msg = "玩家[" + pplayer->getName() + "]说：" + msg;
		broadcast(pplayer->getCurrRoom(), msg.c_str());
	}
	CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
}

std::string roomCardsToString(roomID_t rid)
{
	ostringstream out;
	Room *proom = server().getRoom(rid);
	if (proom == nullptr)
		out << "未加入房间";
	else if (proom->getStatus() != roomStatus_t::playing)
		out << "当前房间未开始游戏";
	else
	{
		for (auto pid : proom->getPlayerIDList())
		{
			Player *pplayer = server().getPlayer(pid);
			if (proom->getLandID() != 0)
			{
				if (proom->getLandID() == pid)
					out << "地主";
				else
					out << "农民";
			}
			out << '[' << pplayer->getName() << "]\t剩余手牌数量：" << proom->getPlayerCardsSize(pid) << endl;
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
		case _S:str = "小";break;
		case _B:str = "大"; break;
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
	case c1:			str = "单牌"; break;
	case c2:			str = "对子"; break;
	case c3:			str = "三张"; break;
	case c4:			str = "炸弹"; break;
	case c11:			str = "王炸"; break;
	case c31:			str = "三带一"; break;
	case c32:			str = "三带二"; break;
	case c411:			str = "四带二"; break;
	case c422:			str = "四带两对"; break;
	case c123:			str = "顺子"; break;
	case c1122:			str = "连对"; break;
	case c111222:		str = "飞机"; break;
	case c11122234:		str = "飞机带翅膀"; break;
	case c1112223344:	str = "飞机带翅膀"; break;
	case c0:			str = "不能出牌"; break;
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
		else if (src.substr(i, i + 4) == "大王") {
			i += 3;
			temp = _B;
		}
		else if (src.substr(i, i + 2) == "大"){
			i += 1;
			temp = _B;
		}
		else if (src.substr(i, i + 4) == "小王") {
			i += 3;
			temp = _S;
		}
		else if (src.substr(i, i + 2) == "小") {
			i += 1;
			temp = _S;
		}
		else if (src.substr(i, i + 4) == "王炸") {
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
	broadcast(pLand->getID(), "玩家[" + pLand->getName() + "]成为了地主！");
	out << "你成为了地主！手牌：\n";
	out << CardsToString(proom->getPlayerCards(pLand->getID()));
	string temp;
	for (auto i : proom->getLandCards())
		temp += CardToString(i);
	broadcast(proom->getID(), "地主牌：" + temp);
	CQ_sendPrivateMsg(ac, PIDToQQ(pLand->getID()), out.str().c_str());
	NotifyNextPlayer(proom->getID(), "轮到你出牌", "等待地主出牌");
}

// 通知该玩家所有人
// 参数 msg      是发送给该轮轮到的玩家的消息
// 参数 otherMsg 是发送给其它玩家的消息
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


// 游戏指令
void gameCommand(uint64_t fromQQ, string msg)
{
	ostringstream out;
	playerID_t pid = QQToPID(fromQQ);
	Player *pplayer = server().getPlayer(pid);
	roomID_t rid = pplayer->getCurrRoom();
	Room * proom = server().getRoom(rid);
do {/********************  do while 块 方便跳出  ************************/
	if (pplayer == nullptr ||
		proom == nullptr ||
		proom->getStatus() != roomStatus_t::playing)
	{
		out << "当前状态无法使用游戏指令，必须在游戏状态下才可以使用";
		break;
	}
	if (msg == "看牌")
	{
		out << roomCardsToString(rid);
		out << CardsToString(proom->getPlayerCards(pid));
		break;
	}
	if (msg == "叫" || msg == "不叫" || msg == "叫地主")
	{
		if (proom->getLandID() != 0)
		{
			out << "已经过了争地主的阶段！";
			break;
		}
		if (proom->getThisTurn() != pid)
		{
			out << "还没轮到你哦！";
			break;
		}
		if (proom->isColledLand())
		{
			out << "现在是抢地主的阶段！";
			break;
		}
		if (msg == "叫" || msg == "叫地主")
		{
			proom->coll(true);
			broadcast(pid, "玩家["+pplayer->getName()+"]叫地主");
			
			if (proom->getLandID() == pid)
			{
				becomeLandlord(proom);
			}
			else
			{
				NotifyNextPlayer(rid, "是否抢地主 [抢地主] [不抢]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]抢地主");
			}
		}
		else
		{
			if (!proom->coll(false)) // 如果返回假表示没人叫地主，自动重开游戏
			{
				broadcast(rid, "所有人都没叫地主，本局重开");
				// 给所有玩家看牌
				for (auto pid : proom->getPlayerIDList())
					CQ_sendPrivateMsg(ac, PIDToQQ(pid), CardsToString(proom->getPlayerCards(pid)).c_str());
			}
			else
			{
				out << "你不叫地主";
				broadcast(pid, "玩家[" + pplayer->getName() + "]不叫地主");
			}
			NotifyNextPlayer(rid, "是否叫地主 [叫地主] [不叫]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]叫地主");
		}
	}
	else if (msg == "抢" || msg == "不抢" 
		|| msg == "我抢" || msg == "抢地主")
	{
		if (proom->getLandID() != 0)
		{
			out << "已经过了争地主的阶段！";
			break;
		}
		if (proom->getThisTurn() != pid)
		{
			out << "还没轮到你哦！";
			break;
		}
		if (!proom->isColledLand())
		{
			out << "现在是叫地主的阶段！";
			break;
		}
		if (msg == "抢" || msg == "我抢" ||  msg == "抢地主")
		{
			broadcast(pid, "玩家[" + pplayer->getName() + "]抢地主");
			if (proom->grab(true))
			{
				becomeLandlord(proom);
			}
			else {
				NotifyNextPlayer(rid, "是否抢地主 [抢地主] [不抢]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]抢地主");
			}
		}
		else
		{
			broadcast(pid, "玩家[" + pplayer->getName() + "]不抢地主");
			if (proom->grab(false))
			{
				becomeLandlord(proom);
			}
			else {
				NotifyNextPlayer(rid, "是否抢地主 [抢地主] [不抢]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]抢地主");
			}
		}
	}
	else if (msg.substr(0, 2) == "出")
	{
		if (proom->getLandID() == 0)
		{
			out << "现在还是争地主的阶段！";
			break;
		}
		if (proom->getThisTurn() != pid)
		{
			out << "还没轮到你哦！";
			break;
		}
		vector<card_t> vec = StringToCards(msg.substr(2));
		PlayingCards pc(vec);
		logFile << CardsToString(vec) << std::endl;
		if (pc.getType() == c0)
		{
			out << "你出的牌不符合规则！";
		}
		if (proom->popCards(pc))
		{
			ostringstream outTemp;
			outTemp << "玩家[";
			outTemp << pplayer->getName();
			outTemp << "]打出了[";
			outTemp << CardTtypeToString(pc.getType());
			outTemp << "]\n";
			outTemp << CardsToString(vec);
			broadcast(rid, outTemp.str());
			outTemp.str("");
			if (proom->isEndGame())
			{
				outTemp << "游戏结束！" << std::endl;;
				if (pplayer->getID() == proom->getLandID())
					outTemp << "地主胜利！" << std::endl;
				else
					outTemp << "农民胜利！" << std::endl;
				for (auto pid : proom->getPlayerIDList())
				{
					int score = proom->settle(pid);
					Player *pPlayerTemp = server().getPlayer(pid);
					pPlayerTemp->score(score);
					outTemp << "玩家[" << pPlayerTemp->getName() << "] 得分:" << score << std::endl;
				}
				outTemp << "发送 查看房间 来查看当前房间情况\n发送 查看 来查看当前个人信息";
				broadcast(rid, outTemp.str());
				return;
			}

			if (proom->isPopCards())
				NotifyNextPlayer(rid, "请出牌 [出 xxx]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]出牌");
			else
				NotifyNextPlayer(rid, "请出牌 [出 xxx] [不出]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]出牌");
		}
		else
		{
			out << "你出的牌没有上家大！上家出的牌：\n";
			out << CardsToString(proom->getLastRoundCards());
		}
	}
	else if (msg == "不出" || msg == "不要" || msg == "要不起")
	{
		if (proom->getLandID() == 0)
		{
			out << "现在还是争地主的阶段！";
			break;
		}
		if (proom->getThisTurn() != pid)
		{
			out << "还没轮到你哦！";
			break;
		}
		if (proom->pass())
		{
			broadcast(rid, "玩家[" + pplayer->getName() + "]不出");
			if (proom->isPopCards())
				NotifyNextPlayer(rid, "请出牌 [出 xxx]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]出牌");
			else
				NotifyNextPlayer(rid, "请出牌 [出 xxx] [不出]", "等待[" + server().getPlayer(proom->getThisTurn())->getName() + "]出牌");
		}
		else
		{
			out << "现在是出牌阶段，无法pass，请出牌！";
		}
	}
	
	else if (msg == "强制重开")
	{
		logFile << "强制重开游戏";
		broadcast(rid, "强制重开游戏，发送 看牌 查看手牌");
		proom->start();
	}
	else
	{
		msg = "玩家[" + pplayer->getName() + "]说：" + msg;
		broadcast(rid, msg.c_str());
		break;
	}
}while(0);/**************  do while 块 方便跳出  ****************/
	CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
}

void Receive(uint64_t fromQQ, string msg)
{
	ostringstream out;
	playerID_t pid = QQToPID(fromQQ);
	Player *pplayer = server().getPlayer(pid);
	if (msg.substr(0, 4) == "注册" && pplayer == nullptr)
	{
		string name;
		istringstream in(msg.substr(4));
		in >> name;
		Player temp(pid);
		temp.setName(name.substr(0, 12));
		if (server().loginPlayer(temp))
		{
			out << "注册成功！昵称：" << name << std::endl;
			out << "发送 菜单 来得到指令列表";
		}
		else {
			out << "注册失败！";
		}
		CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
		return;
	}
	if (pplayer == nullptr)
	{
		out << "当前QQ未注册，请回复 注册 昵称 来完成注册！" << std::endl;
		out << "例子：注册 隔壁老王" << std::endl;
		out << "请注意，昵称最长6个汉字或者12个字母,注册后面跟名字,名字中不允许有空格";
		CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
		return;
	}
	Room *proom = server().getRoom(pplayer->getCurrRoom());
	if (msg == "菜单")
	{
		out <<
			"+--------指令列表--------+\n"
			"| 菜单 -- 输出指令列表   |\n"
			"| 注册 -- 注册 昵称      |\n"
			"| 改名 -- 改名 新昵称    |\n"
			"| 查看 -- 查看当前信息   |\n"
			"| 更新内容 -- 尾更新内容 |\n"
			"| [消息] -- 在房间中聊天 |\n"
			"|----    大厅指令    ----|\n"
			"|准备 -- 若已准备，则取消|\n"
			"|创建房间 -- 创建一个房间|\n"
			"|加入[id] -- 加入指定房间|\n"
			"|退出房间 -- 退出当前房间|\n"
			"|查看房间 -- 查看当前房间|\n"
			"|房间列表 -- 查看所有房间|\n"
			"|玩家列表 -- 查看所有玩家|\n"
			"|*注意大厅指令游戏中禁用*|\n"
			"|----    游戏指令    ----|\n"
			"|看牌 -- 查看当前牌局情况|\n"
			"|叫地主/不叫  抢地主/不抢|\n"
			"|出牌/不出    出 xxxx    |\n"
			"|*注意游戏指令大厅中禁用*|\n"
			"+--------指令列表--------+";
	}
	else if (msg == "查看")
	{
		out << playerInfoToString(pid);
	}
	else if (msg.substr(0, 4) == "改名")
	{
		string name;
		istringstream in(msg.substr(4));
		in >> name;
		pplayer->setName(name.substr(0, 12));
		out << "修改完成，当前昵称:" << name;
	}
	else if (msg == "更新内容")
	{
		out << "正在做从文件中读更新内容的功能，敬请期待";
	}
	else if (proom == nullptr || proom->getStatus() != roomStatus_t::playing)
		instruction(fromQQ, msg);
	else if (proom->getStatus() == roomStatus_t::playing)
		gameCommand(fromQQ, msg);
	else
		out << "发送 菜单 来得到指令列表";
	CQ_sendPrivateMsg(ac, fromQQ, out.str().c_str());
}