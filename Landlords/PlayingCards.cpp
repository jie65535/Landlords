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
	else if (size < 5) // ���С��5��
	{
		// ��β��ͬ  �� ˫ �� ը
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
		else if (size == 4) // ���Ų�ͬ��  ����һ?
		{
			if (cards[0] == cards[2])
				type = c31, max = cards[0];
			else if (cards[1] == cards[3])
				type = c31, max = cards[1];
			else
				type = c0;
		}
	}
	else if (size >= 5) // ������ڵ���5��
	{
		// std::vector<card_t> cards; Ҫʶ�����
		// arr[0] �ظ�һ�ε���
		// arr[1] �ظ����ε���
		// arr[2] �ظ����ε���
		// arr[3] �ظ��Ĵε���
		std::vector<card_t> arr[4];
		for (int index = 0; index < size;)
		{
			card_t temp = cards[index++];
			int count = 1;
			for (; index < size && temp == cards[index]; ++index)
				count++;
			if (temp == _S || temp == _B && count > 1) // ����С�����ж�ģ�
				type = c0; 
			if (count > 4) // ��������һ���ģ���
				type = c0;
			else
				arr[count - 1].push_back(temp);
		}

		// ����һ��
		if (size == 5 && arr[2].size() == 1 && arr[1].size() == 1)
			type = c32, max = arr[2].front();
		// �Ĵ����� 333345��666677
		else if (size == 6 && arr[3].size() == 1)
			type = c411, max = arr[3].front();
		// �Ĵ�����
		else if (size == 8 && arr[3].size() == 1 && arr[1].size() == 2)
			type = c422, max = arr[3].front();
		// ��˳
		else if (size == arr[0].size()
			&& arr[0].back() - arr[0].front() == size - 1 // ���һ�ż���һ�� == ����-1
			&& arr[0].back() < _2)                        // ������С��2
		{
			type = c123, max = arr[0].back();
		}
		// ����
		else if (size / 2 >= 3 && (size % 2 == 0) // �ж��Ƿ����3�Բ���������ż��
			&& arr[1].size() == size / 2            // �ǲ��Ƕ��Ƕ���
			&& arr[1].back() - arr[1].front() == size / 2 - 1  // �����ǲ������ŵ�
			&& arr[1].back() < _2) // ������С��2
		{
			type = c1122, max = arr[1].back();
		}
		// �ɻ�
		else if (arr[2].size() == size / 3
			&& (size % 3 == 0)
			&& arr[2].back() - arr[2].front() == (size / 3 - 1)
			&& arr[2].back() < _2)
		{
			type = c111222, max = arr[2].back();
		}
		// �ɻ������ ��
		else if (arr[2].size() >= 2
			&& arr[2].size() == size / 4
			&& (size % 4 == 0)
			&& arr[2].back() - arr[2].front() == size / 4 - 1
			&& arr[2].back() < _2)
		{
			type = c11122234, max = arr[2].back();
		}
		// �ɻ������ ˫
		else if (arr[2].size() >= 2
			&& arr[2].size() == size / 5
			&& (size % 5 == 0)                                 // ��5������
			&& arr[2].back() - arr[2].front() == size / 5 - 1  // �Ƿ���
			&& arr[1].size() == size / 5   // ����ͬ�����Ķ���
			&& arr[2].back() < _2)
		{
			type = c1112223344, max = arr[2].back();
		}
		// ʲô�����ǣ����ܳ���
		else
			type = c0;

		// ���ƽ���������ͬ���������ǰ
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
	} // ������ڵ���5�� -- end

}

bool PlayingCards::operator<(const PlayingCards & rhs) const
{
	// ��ը
	if (type == c11)
		return false;
	else if (rhs.type == c11)
		return true;

	// ը��
	if (type == c4 && rhs.type != c4)
		return false;
	else if (type != c4 && rhs.type == c4)
		return true;

	// �ٶ���������һ�����Ƚ�Ȩֵ
	return max < rhs.max;
}
