#include <stdio.h>
#include <string.h>
#include <utility>
using std::swap;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int kQueenKindNumber = 3;
const int kMaxQueenNumber = 50;
// [0][column]: false if no queen in this column; otherwise true.
// [1][row + column] and [2][row - column + queen_number] are for 2 kinds of
// diagonals(/ and \): false if no queen in this diagonal; otherwise true.
// (row + column) & (row - column + queen_number) can be calculated by
// formula: y = k*x + b, which y(row), k(1 or -1), x(column) and b is a constant.
// Since row - column can be negative, so we add queen_number to
// guarantee that the index is nonnegative.
bool have_queen[kQueenKindNumber][kMaxQueenNumber * 2 + 10];
// position[row] = column: place queen in [row][column]. Used to print solution.
int position[kMaxQueenNumber];
int solution_number;
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
void NQueenBacktrack(int row, const int queen_number)
{
	if(row == queen_number)
	{
		//PrintQueen(queen_number);
		++solution_number;
	}
	else
	{
		for(int column = 0; column < queen_number; ++column)
		{
			if(have_queen[0][column] == true ||
			        have_queen[1][row + column] == true ||
			        have_queen[2][row - column + queen_number] == true)
			{
				continue;
			}
			position[row] = column;
			have_queen[0][column] = have_queen[1][row + column] = have_queen[2][row - column + queen_number] = true;
			NQueenBacktrack(row + 1, queen_number);
			have_queen[0][column] = have_queen[1][row + column] = have_queen[2][row - column + queen_number] = false;
		}
	}
}
void NQueen(int queen_number)
{
	memset(have_queen, false, sizeof have_queen);
	solution_number = 0;
	NQueenBacktrack(0, queen_number);
}
void TestNQueen()
{
	printf("----------TestNQueen----------\n");
	const int kCaseNumber = 10;
	int queen_number[kCaseNumber] = {1,2,3,4,5,6,7,8,9,10};
	int answer[kCaseNumber] = {1,0,0,2,10,4,40,92,352,724};
	for(int index = 0; index < kCaseNumber; ++index)
	{
		NQueen(queen_number[index]);
		if(solution_number != answer[index])
		{
			printf("Case %d Not pass.\n", index);
		}
	}
	printf("All Case pass.\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int kBoardSize = 9;
const int kPuzzleSize = kBoardSize * kBoardSize + 1;
bool row_flag[kBoardSize][kBoardSize];
bool column_flag[kBoardSize][kBoardSize];
bool area_flag[kBoardSize][kBoardSize];
void SetFlag(int row, int column, int area, int number, bool value)
{
	row_flag[row][number] = value;
	column_flag[column][number] = value;
	area_flag[area][number] = value;
}
bool IsPlaced(int row, int column, int area, int number)
{
	return row_flag[row][number] ||
	       column_flag[column][number] ||
	       area_flag[area][number];
}
void InitializeFlag(char(&board)[kBoardSize][kBoardSize])
{
	memset(row_flag, false, sizeof row_flag);
	memset(column_flag, false, sizeof column_flag);
	memset(area_flag, false, sizeof area_flag);
	for(int row = 0; row < kBoardSize; ++row)
	{
		for(int column = 0; column < kBoardSize; ++column)
		{
			if(board[row][column] == '0')
			{
				continue;
			}
			int area = row / 3 * 3 + column / 3;
			int number = board[row][column] - '0' - 1;
			SetFlag(row, column, area, number, true);
		}
	}
}
bool SudokuBacktrack(char(&board)[kBoardSize][kBoardSize], int row, int column)
{
	if(row >= kBoardSize)
	{
		return true;
	}
	if(column >= kBoardSize)
	{
		return SudokuBacktrack(board, row + 1, 0);
	}
	if(board[row][column] != '0')
	{
		return SudokuBacktrack(board, row, column + 1);
	}

	int area = row / 3 * 3 + column / 3;
	for(int number = 0; number < kBoardSize; ++number)
	{
		if(IsPlaced(row, column, area, number) == true)
		{
			continue;
		}
		board[row][column] = static_cast<char>('0' + number + 1);
		SetFlag(row, column, area, number, true);
		if(SudokuBacktrack(board, row, column + 1) == true)
		{
			return true;
		}
		board[row][column] = '0';
		SetFlag(row, column, area, number, false);
	}
	return false;
}
void Sudoku(char *puzzle)
{
	char board[kBoardSize][kBoardSize];
	for(int row = 0; row < kBoardSize; ++row)
	{
		for(int column = 0; column < kBoardSize; ++column)
		{
			board[row][column] = puzzle[row * kBoardSize + column];
		}
	}
	InitializeFlag(board);
	if(SudokuBacktrack(board, 0, 0) == true)
	{
		for(int row = 0; row < kBoardSize; ++row)
		{
			for(int column = 0; column < kBoardSize; ++column)
			{
				puzzle[row * kBoardSize + column] = board[row][column];
			}
		}
		return;
	}
	*puzzle = 0;
}
void TestSudoku()
{
	printf("----------TestSudoku----------\n");
	const int kCaseNumber = 3;
	char sudoku_puzzle[kCaseNumber][kPuzzleSize] =
	{
		"000000010400000000020000000000050407008000300001090000300400200050100000000806000",
		"000000010400000000020000000000050407008000300001090000300400200050100000000806005",
		"693784512487512936125963874932651487568247391741398625319475268856129743274836159",
	};
	char sudoku_answer[kCaseNumber][kPuzzleSize] =
	{
		"693784512487512936125963874932651487568247391741398625319475268856129743274836159",
		"\0",
		"693784512487512936125963874932651487568247391741398625319475268856129743274836159",
	};
	for(int index = 0; index < kCaseNumber; ++index)
	{
		Sudoku(sudoku_puzzle[index]);
		if(strcmp(sudoku_puzzle[index], sudoku_answer[index]) != 0)
		{
			printf("Case %d Not pass.\n", index);
		}
	}
	printf("All Case pass.\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GetPermutation(int *data, int first, int last)
{
	for(int index = first; index < last; ++index)
	{
		printf("%d ", data[index]);
	}
	printf("\n");
}
int g_permutation_number = 0;
void Permutation(int *data, int first, int last)  // [first, last)
{
	if(last - first == 1) // Length = 1, recursive ends, get one n-data permutation.
	{
		++g_permutation_number;
		GetPermutation(data, 0, last);
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
	printf("----------TestPermutationAndSubset----------\n");
	const int kCaseNumber = 4, kDataLength = 4;
	int data[kCaseNumber][kDataLength] = {{1,2,3,4}, {1,1,2,3}, {1,1,1,2},{1,1,1,1}};
	for(int index = 0; index < kCaseNumber; ++index)
	{
		g_permutation_number = 0;
		Permutation(data[index], 0, kDataLength);
		printf("Case %d: total %d permutation.\n", index, g_permutation_number);
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
	TestNQueen();
	TestSudoku();
	TestPermutationAndSubset();
}
