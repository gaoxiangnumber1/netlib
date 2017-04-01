#include<iostream>
using namespace std;
const int INF = -100000000;

int DivideAndConquer(int test[], int left, int right);
int DynamicProgramming(int test[], int length);

int main()
{
	int test[100];
	while(1)
	{
		int length;
		cin >> length;
		for(int index = 0; index < length; index++)
		{
			cin >> test[index];
		}
		cout << "*****************************************************\n";
		cout << "DivideAndConquer:   " << DivideAndConquer(test, 0, length - 1) << "\n";
		cout << "DynamicProgramming: " << DynamicProgramming(test, length) << "\n";
	}

	return 0;
}

// get the max sum of [left, right] in array test.
int DivideAndConquer(int test[], int left, int right)
{
	if(left == right)  // if there is only 1 element, no need to divide
	{
		return test[left];
	}
	int middle = left + (right - left) / 2;  // divide position
	int max_left_sum = DivideAndConquer(test, left, middle);  // get max sum in left-sub-array
	int max_right_sum= DivideAndConquer(test, middle + 1, right);  // get max sum in right-sub-array
	// get the max sum which cross from left to right
	// first get max-left-part-sum which add from middle to left: [left, middle]
	int left_border_sum = 0, max_left_border_sum = INF;
	for(int index = middle; index >= left; index--)
	{
		left_border_sum = left_border_sum + test[index];
		max_left_border_sum = max(max_left_border_sum, left_border_sum);
	}
	// second get max-right-part-sum which add from middle to right: [middle + 1, right]
	int right_border_sum = 0, max_right_border_sum = INF;
	for(int index = middle + 1; index <= right; index++)
	{
		right_border_sum = right_border_sum + test[index];
		max_right_border_sum = max(max_right_border_sum, right_border_sum);
	}
	// return biggest sum from 3 max sum.
	return max(max_left_sum,
				max(max_right_sum, max_left_border_sum + max_right_border_sum));
}

int DynamicProgramming(int test[], int length)
{
	// max_sum is the result, that is, it is the biggest value among current_max_sum
	// current_max_sum is the max successive sum end with current element(test[index])
	int max_sum = INF, current_max_sum = INF;
	for(int index = 0; index < length; index++)  // traversal the array
	{
		// at this point: current_max_sum is the max successive sum end with test[index-1]
		// so if current_max_sum is not-positive, then
		// current_max_sum + test[index] <= test[index]
		// so we make current_max_sum = test[index]; otherwise continue add.
		if(current_max_sum <= 0)
		{
			current_max_sum = test[index];
		}
		else
		{
			current_max_sum = current_max_sum + test[index];
		}
		// up to now, current_max_sum is the max successive sum end with test[index]
		// and we update max_sum
		max_sum = max(max_sum, current_max_sum);
	}
	return max_sum;
}

/*
6
-2 11 -4 13 -5 -2
10
-10 1 2 3 4 -5 -23 3 7 -21
6
5 -8 3 2 5 0
1
10
3
-1 -5 -2
3
-1 0 -2

*/

/*
20
10
10
10
-1
0
*/
