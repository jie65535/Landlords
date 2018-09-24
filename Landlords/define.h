#pragma once

#include <cstdint>
#include <fstream>
#include <random>

using playerID_t = uint64_t;
using roomID_t   = uint32_t;

extern std::ofstream logFile;
// �����������
extern std::default_random_engine e;
// �ֲ�����
extern std::uniform_int_distribution<int> landU;        // ѡ����

extern std::uniform_int_distribution<int> cardsU;       // ϴ��

enum card_t {
	_3 = 1,
	_4,
	_5,
	_6,
	_7,
	_8,
	_9,
	_10,
	_J,
	_Q,
	_K,
	_A,
	_2,
	_S,
	_B
};
