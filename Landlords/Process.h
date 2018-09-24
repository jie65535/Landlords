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

// 广播消息给指定玩家ID的房间（该消息将发送给指定玩家之外所有人）
void broadcast(playerID_t pid, const std::string& msg);

// 广播消息给指定房间ID的房间（该消息将发送给该房间所有人）
void broadcast(roomID_t rid, const std::string& msg);


void instruction(uint64_t fromQQ, std::string msg);

std::string roomCardsToString(roomID_t rid);

std::string CardToString(card_t card);

std::string CardsToString(const std::vector<card_t> &cards);
void becomeLandlord(Room * proom);

// 通知该玩家所有人
// 参数 msg      是发送给该轮轮到的玩家的消息
// 参数 otherMsg 是发送给其它玩家的消息
void NotifyNextPlayer(roomID_t rid, std::string msg, std::string otherMsg);
void gameCommand(uint64_t fromQQ, std::string msg);

void Receive(uint64_t fromQQ, std::string msg);