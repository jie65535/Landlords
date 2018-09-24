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

	// ��δ�ڷ��䣬���ؼ١����Ѿ�׼������ȡ��׼������δ׼������׼��
	bool playerReady(playerID_t pid);

	bool startGame(roomID_t rid);
private:
	// �����Ƿ������˶�׼��
	bool isRoomAllReady(Room* proom);
	
private:
	std::vector<Player> m_playerList;
	std::vector<Room>   m_roomList;
};

Server &server();  // ����