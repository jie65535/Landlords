#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "Player.h"
#include "Room.h"
class Room;
class Server
{
public:
	Server();
	~Server();

	Room   * getRoom(roomID_t rid);
	Player * getPlayer(playerID_t pid);

	roomID_t createRoom(playerID_t pid);
	playerID_t loginPlayer(Player player);
	bool joinRoom(playerID_t pid, roomID_t rid);
	bool leaveRoom(playerID_t pid);

	const std::vector<Room> & getRoomList() { return m_roomList; }
	const std::vector<Player> & getPlayerList() { return m_playerList; }

	// 若未在房间，返回假。若已经准备，则取消准备，若未准备，则准备
	bool playerReady(playerID_t pid);

	bool startGame(roomID_t rid);
private:
	// 房间是否所有人都准备
	bool isRoomAllReady(Room* proom);
	
private:
	std::vector<Player> m_playerList;
	std::vector<Room>   m_roomList;
};

Server &server();  // 单例