#pragma once
#include "cqp.h"
#include "Server.h"
#include "Room.h"
#include "Player.h"
#include <string>
#include <vector>
#include <sstream>
std::string playerInfoToString(Player* pplayer);
std::string playerInfoToString(playerID_t pid);
std::string playerListToString();
std::string roomInfoToString(roomID_t rid);

std::string roomListToString(bool isHideFull);

inline playerID_t QQToPID(uint64_t fromQQ);

inline uint64_t PIDToQQ(playerID_t pid);

// �㲥��Ϣ��ָ�����ID�ķ��䣨����Ϣ�����͸�ָ�����֮�������ˣ�
void broadcast(playerID_t pid, const std::string& msg);

// �㲥��Ϣ��ָ������ID�ķ��䣨����Ϣ�����͸��÷��������ˣ�
void broadcast(roomID_t rid, const std::string& msg);


void instruction(uint64_t fromQQ, std::string msg);

std::string roomCardsToString(roomID_t rid);

std::string CardToString(card_t card);

std::string CardsToString(const std::vector<card_t> &cards);
void becomeLandlord(Room * proom);

// ֪ͨ�����������
// ���� msg      �Ƿ��͸������ֵ�����ҵ���Ϣ
// ���� otherMsg �Ƿ��͸�������ҵ���Ϣ
void NotifyNextPlayer(roomID_t rid, std::string msg, std::string otherMsg);
void gameCommand(uint64_t fromQQ, std::string msg);

void Receive(uint64_t fromQQ, std::string msg);