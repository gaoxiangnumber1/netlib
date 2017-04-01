#include<iostream>
#include<cstring>  // to use memset
using namespace std;

int queen_number, solution_number;
// visit[0][column] = 0 if there is no queen in this column; otherwise = 1;
// visit[1][row + column] and visit[2][row - column + queen_number] are for 2
// kinds of diagonals(/ and \), = 0 if no queen on this line; otherwise = 1.
// row + column & row - column + queen_number can be easily calculated
// by formula: y = k*x + b, which y(row), k(1 or -1), x(column) and b is a constant.
// Note: row - column can be negative, so we add queen_number to guarantee that
// the index is nonnegative.
int visit[3][100];
// position[row] = column: queen in row "row" should be placed in column "column"
// this array is not essential if you don't print solutions.
int position[100];

void Output();
void BackTrack(int row);

int main()
{
	while(1)
	{
		memset(visit, 0, sizeof(visit));  // set all numbers in visit to 0
		solution_number = 0;
		cout << "Please input queen_number:\n";
		cin >> queen_number;
		BackTrack(0);  // start place queens from [0]row
		cout << "Total solutions number is " << solution_number << endl;
	}
}

void BackTrack(int row)
{
	if(row == queen_number)
	// we should place queens in [0, queen_number - 1]row, so row equals to queen_number
	// indicates we have placed all queens, so get a solution and stop backtracking.
	{
		solution_number++;
		Output();
	}
	else
	{
		// for current row "row", check each column whether can place queen by "visit"
		// and if can, change according element in "visit" and continue placing queen
		// in next row "row + 1".
		for(int column = 0; column < queen_number; column++)
		{
			if(visit[0][column] == 0 && visit[1][row + column] == 0 &&
			visit[2][row - column + queen_number] == 0)
			{
				position[row] = column;
				visit[0][column] = visit[1][row + column] = visit[2][row - column + queen_number] = 1;
				BackTrack(row + 1);
				// after backtrack return, we must retrieve "visit" because we will place queen
				// in another column for current row by "column++".
				visit[0][column] = visit[1][row + column] = visit[2][row - column + queen_number] = 0;
			}
		}
	}
}

void Output()
{
	for(int row = 0; row < queen_number; row++)
	{
		for(int column = 0; column < queen_number; column++)
		{
			// 1 stand for has queen; otherwise 0
			cout << ((position[row] == column) ? "1 " : "0 ");
		}
		cout << endl;
	}
	cout << endl;
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
*/
