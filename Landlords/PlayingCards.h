#pragma once
#include "define.h"
#include <vector>

enum CardType {
	c1,			//����
	c2,			//����
	c3,			//3����
	c4,			//ը��
	c11,		//��ը
	c31,		//3��1
	c32,		//3��2
	c411,		//4��2����������һ��
	c422,		//4��2��
	c123,		//����
	c1122,		//����
	c111222,	//�ɻ�
	c11122234,	//�ɻ�������
	c1112223344,//�ɻ�������
	c0			//���ܳ���
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
	// �Ƚϴ�С�������Ƿ���
	bool operator<(const PlayingCards& rhs) const;
private:
	CardType type; // ����
	card_t max;    // Ȩֵ
	int size;   // �Ƶ�����
};