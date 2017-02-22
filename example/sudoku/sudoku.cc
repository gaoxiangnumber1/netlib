#include "sudoku.h"

#include <stdio.h>

using std::string;

__thread bool row_mask[9][9], column_mask[9][9], area_mask[9][9];

inline void SetMask(int row, int column, int area, int number, bool value)
{
	row_mask[row][number] = value;
	column_mask[column][number] = value;
	area_mask[area][number] = value;
}
inline bool HaveTrue(int row, int column, int area, int number)
{
	return row_mask[row][number] ||
	       column_mask[column][number] ||
	       area_mask[area][number];
}
bool InitializeMask(char(&board)[9][9])
{
	for(int index = 0; index < 9; ++index)
	{
		for(int number = 0; number < 9; ++number)
		{
			SetMask(index, index, index, number, false);
		}
	}
	for(int row = 0; row < 9; ++row)
	{
		for(int column = 0; column < 9; ++column)
		{
			if(board[row][column] == '0') // NOTE
			{
				continue;
			}
			int area = (row / 3) * 3 + column / 3;
			int number = board[row][column] - '0' - 1;
			if(HaveTrue(row, column, area, number) == true)
			{
				return false;
			}
			SetMask(row, column, area, number, true);
		}
	}
	return true;
}
bool BackTrack(char(&board)[9][9], int row, int column)
{
	if(row >= 9)
	{
		return true;
	}
	if(column >= 9)
	{
		return BackTrack(board, row + 1, 0);
	}
	if(board[row][column] != '0')
	{
		return BackTrack(board, row, column + 1);
	}

	int area = (row / 3) * 3 + (column / 3);
	for(int number = 0; number < 9; ++number)
	{
		if(HaveTrue(row, column, area, number) == true)
		{
			continue;
		}
		board[row][column] = static_cast<char>('0' + number + 1);
		SetMask(row, column, area, number, true);
		if(BackTrack(board, row, column + 1) == true) // NOTE
		{
			return true;
		}
		board[row][column] = '0';
		SetMask(row, column, area, number, false);
	}
	return false;
}
string SolveSudoku(const string puzzle)
{
	char board[9][9];
	for(int row = 0; row < 9; ++row)
	{
		for(int column = 0; column < 9; ++column)
		{
			board[row][column] = puzzle[row * 9 + column];
		}
	}
	if(InitializeMask(board) == true && BackTrack(board, 0, 0) == true) // NOTE
	{
		string result = puzzle;
		for(int row = 0; row < 9; ++row)
		{
			for(int column = 0; column < 9; ++column)
			{
				result[row * 9 + column] = board[row][column];
			}
		}
		return result;
	}
	return "NoSolution!";
}
