#pragma once
#include "define.h"
#include <vector>

enum CardType {
	c1,			//单牌
	c2,			//对子
	c3,			//3不带
	c4,			//炸弹
	c11,		//王炸
	c31,		//3带1
	c32,		//3带2
	c411,		//4带2个单，或者一对
	c422,		//4带2对
	c123,		//连子
	c1122,		//连队
	c111222,	//飞机
	c11122234,	//飞机带单排
	c1112223344,//飞机带对子
	c0			//不能出牌
};



class PlayingCards
{
public:
	PlayingCards() = delete;
	PlayingCards(const std::vector<card_t>& incards):cards(incards) { updata(); };
	void updata();
	std::vector<card_t> cards;
	const std::vector<card_t> & getVec() const { return cards; }
	CardType getType() const { return type; }
	int getSize() const { return size; }
	// 比较大小，不检测非法！
	bool operator<(const PlayingCards& rhs) const;
private:
	CardType type; // 牌型
	card_t max;    // 权值
	int size;   // 牌的数量
};