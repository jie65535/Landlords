#include "stdafx.h"
#include "PlayingCards.h"

void PlayingCards::updata()
{
	if (cards.empty())
	{
		type = c0;
		return;
	}
	std::sort(cards.begin(), cards.end());
	size = cards.size();
	if (size == 2 && cards[0] == _S && cards[1] == _B)
	{
		type = c11;
	}
	else if (size < 5) // 如果小于5张
	{
		// 首尾相同  单 双 三 炸
		if (cards.front() == cards.back())
		{
			if (size == 1)
				type = c1;
			else if (size == 2)
				type = c2;
			else if (size == 3)
				type = c3;
			else if (size == 4)
				type = c4;
			max = cards[0];
		}
		else if (size == 4) // 四张不同的  三带一?
		{
			if (cards[0] == cards[2])
				type = c31, max = cards[0];
			else if (cards[1] == cards[3])
				type = c31, max = cards[1];
			else
				type = c0;
		}
	}
	else if (size >= 5) // 如果大于等于5张
	{
		// std::vector<card_t> cards; 要识别的牌
		// arr[0] 重复一次的牌
		// arr[1] 重复两次的牌
		// arr[2] 重复三次的牌
		// arr[3] 重复四次的牌
		std::vector<card_t> arr[4];
		for (int index = 0; index < size;)
		{
			card_t temp = cards[index++];
			int count = 1;
			for (; index < size && temp == cards[index]; ++index)
				count++;
			if (temp == _S || temp == _B && count > 1) // 大王小王还有多的？
				type = c0; 
			if (count > 4) // 五张以上一样的？？
				type = c0;
			else
				arr[count - 1].push_back(temp);
		}

		// 三带一对
		if (size == 5 && arr[2].size() == 1 && arr[1].size() == 1)
			type = c32, max = arr[2].front();
		// 四带两张 333345、666677
		else if (size == 6 && arr[3].size() == 1)
			type = c411, max = arr[3].front();
		// 四带两对
		else if (size == 8 && arr[3].size() == 1 && arr[1].size() == 2)
			type = c422, max = arr[3].front();
		// 单顺
		else if (size == arr[0].size()
			&& arr[0].back() - arr[0].front() == size - 1 // 最后一张减第一张 == 总数-1
			&& arr[0].back() < _2)                        // 最大的牌小于2
		{
			type = c123, max = arr[0].back();
		}
		// 连对
		else if (size / 2 >= 3 && (size % 2 == 0) // 判断是否大于3对并且牌数是偶数
			&& arr[1].size() == size / 2            // 是不是都是对子
			&& arr[1].back() - arr[1].front() == size / 2 - 1  // 对子是不是连着的
			&& arr[1].back() < _2) // 最大的牌小于2
		{
			type = c1122, max = arr[1].back();
		}
		// 飞机
		else if (arr[2].size() == size / 3
			&& (size % 3 == 0)
			&& arr[2].back() - arr[2].front() == (size / 3 - 1)
			&& arr[2].back() < _2)
		{
			type = c111222, max = arr[2].back();
		}
		// 飞机带翅膀 单
		else if (arr[2].size() >= 2
			&& arr[2].size() == size / 4
			&& (size % 4 == 0)
			&& arr[2].back() - arr[2].front() == size / 4 - 1
			&& arr[2].back() < _2)
		{
			type = c11122234, max = arr[2].back();
		}
		// 飞机带翅膀 双
		else if (arr[2].size() >= 2
			&& arr[2].size() == size / 5
			&& (size % 5 == 0)                                 // 被5整除？
			&& arr[2].back() - arr[2].front() == size / 5 - 1  // 是否连
			&& arr[1].size() == size / 5   // 有相同数量的对子
			&& arr[2].back() < _2)
		{
			type = c1112223344, max = arr[2].back();
		}
		// 什么都不是，不能出！
		else
			type = c0;

		// 对牌进行排序，相同数量多的在前
		if (type != c0)
		{
			int index = 0;
			for (int r = 3; r >= 0; --r)
			{
				for (int i = 0; i < arr[r].size(); ++i)
				{
					for (int j = 0; j < r+1; ++j)
						cards[index++] = arr[r][i];
				}
			}
			
		}
	} // 如果大于等于5张 -- end

}

bool PlayingCards::operator<(const PlayingCards & rhs) const
{
	// 王炸
	if (type == c11)
		return false;
	else if (rhs.type == c11)
		return true;

	// 炸弹
	if (type == c4 && rhs.type != c4)
		return false;
	else if (type != c4 && rhs.type == c4)
		return true;

	// 假定牌型数量一样，比较权值
	return max < rhs.max;
}
