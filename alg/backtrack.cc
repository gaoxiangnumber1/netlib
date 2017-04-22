#include <stdio.h>
#include <utility>
using std::swap;

void NQueenBacktrack(int row,
                     const int queen_number,
                     int *position,
                     int &solution_number)
{
	if(row == queen_number)
	{
		++solution_number;
	}
	else
	{
		for(int column = 0; column < queen_number; ++column)
		{
			if(VisitHaveTrue)
			if(visit[0][column] == false &&
			        visit[1][row + column] == false &&
			        visit[2][row - column + queen_number] == false)
			{
				position[row] = column;
				visit[0][column] = visit[1][row + column] = visit[2][row - column + queen_number] = true;
				NQueenBacktrack(row + 1);
				visit[0][column] = visit[1][row + column] = visit[2][row - column + queen_number] = false;
			}
		}
	}
}
void PrintQueen(const int queen_number)
{
	for(int row = 0; row < queen_number; ++row)
	{
		for(int column = 0; column < queen_number; ++column)
		{
			printf("%c ", position[row] == column ? '1' : '0');
		}
		printf("\n");
	}
	printf("\n");
}
int NQueen(int queen_number)
{
	const int kNumberOfLineKind = 3;
	const int kMaxQueenNumber = 50;
	// [0][column]: false if no queen in this column; otherwise true.
	// [1][row + column] and [2][row - column + queen_number] are for 2 kinds of
	// diagonals(/ and \): false if no queen in this diagonal; otherwise true.
	// (row + column) & (row - column + queen_number) can be calculated by
	// formula: y = k*x + b, which y(row), k(1 or -1), x(column) and b is a constant.
	// Since row - column can be negative, so we add queen_number to
	// guarantee that the index is nonnegative.
	bool visit[kNumberOfLineKind][kMaxQueenNumber * 2 + 10];
	memset(visit, false, sizeof visit);
	// position[row] = column: place queen in [row][column]. Used to print solution.
	int position[kMaxQueenNumber];
	int solution_number = 0;
	NQueenBacktrack(0, queen_number, position, solution_number);
	return solution_number;
}

/*
1: 1
2: 0
3: 0
4: 2
5: 10
6: 4
7: 40
8: 92
9: 352
10: 724


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool row_mask[9][9], column_mask[9][9], area_mask[9][9];
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ShowContent(int *data, int first, int last)
{
	for(int index = first; index < last; ++index)
	{
		printf("%d ", data[index]);
	}
	printf("\n");
}*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int g_permutation_number = 0;
void Permutation(int *data, int first, int last)  // [first, last)
{
	if(last - first == 1) // Length = 1, recursive ends, get one n-data permutation.
	{
		++g_permutation_number;
		ShowContent(data, 0, last);
		return;
	}
	for(int index = first; index < last; ++index)
	{
		if(index != first && data[first] == data[index]) // Repeated data, No need swap.
		{
			continue;
		}
		swap(data[first], data[index]); // Get permutation of the first position.
		Permutation(data, first + 1, last); // Get permutation of length n-1 data.
		swap(data[first], data[index]); // Backtrack.
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int g_subset_number = 0;
bool g_exist[128];
void GetSubset(int *data, bool *exist, int last)
{
	for(int index = 0; index < last; ++index)
	{
		if(exist[index] == true)
		{
			printf("%d ", data[index]);
		}
	}
	printf("\n");
}
void Subset(int *data, bool *exist, int first, int last) // [first, last). Can't handle same subset.
{
	if(last - first == 1)
	{
		exist[first] = true;
		GetSubset(data, exist, last);
		exist[first] = false;
		GetSubset(data, exist, last);
		g_subset_number += 2;
		return;
	}
	exist[first] = true; // in subset
	Subset(data, exist, first + 1, last); // get subset of [first + 1, last)
	exist[first] = false;
	Subset(data, exist, first + 1, last);
}
void TestPermutationAndSubset()
{
	const int kCaseNumber = 4, kDataLength = 4;
	int data[kCaseNumber][kDataLength] = {{1,2,3,4}, {1,1,2,3}, {1,1,1,2},{1,1,1,1}};
	for(int index = 0; index < kCaseNumber; ++index)
	{
		g_permutation_number = 0;
		//Permutation(data[index], 0, kDataLength);
		//printf("Case %d: total %d permutation.\n", index, g_permutation_number);
	}
	for(int index = 0; index < kCaseNumber; ++index)
	{
		g_subset_number = 0;
		Subset(data[index], g_exist, 0, kDataLength);
		printf("Case %d: total %d subset.\n", index, g_subset_number);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main()
{
	TestPermutationAndSubset();
}

