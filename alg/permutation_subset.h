#include<iostream>
using namespace std;

bool exist[100];

void Permutation(int test[], int first, int last);
void PrintSubset(int test[], bool exist[], int last);
void Subset(int test[], bool exist[], int first, int last);

int main()
{
	int test[100];
	while(1)
	{
		cout << "Please input your array's size(0 to quit):\n";
		int length;
		cin >> length;
		if(length)
		{
			cout << "Please input " << length << " integer:\n";
			for(int index = 0; index < length; index++)
			{
				cin >> test[index];
			}
			cout << "Permutation:\n";
			Permutation(test, 0, length - 1);
			cout << "Subset:\n";
			Subset(test, exist, 0, length - 1);
		}
		else
		{
			break;
		}
	}

	return 0;
}

void Permutation(int test[], int first, int last)  // get the permutation of test[first, last]
{
	if(first == last)  // no need to swap element, so we get one permutation
	{
		for(int index = 0; index <= last; index++)
		{
			cout << test[index] << " ";
		}
		cout << endl;
	}
	else
	{
		for(int index = first; index <= last; index++)
		{
			swap(test[first], test[index]);  // exchange the first element with all elements one by one
			Permutation(test, first + 1, last);  // get the permutation of test[first + 1, last]
			swap(test[first], test[index]);  // recover the exchange
		}
	}
}

void Subset(int test[], bool exist[], int first, int last)  // get the subset of test[first, last]
{
	if(first == last)  // reach the end
	{
		exist[first] = true;  // the last element is in this subset
		PrintSubset(test, exist, last);  // get one subset
		exist[first] = false;
		PrintSubset(test, exist, last);
		return;
	}
	exist[first] = true;  // the first element is in this subset
	Subset(test, exist, first + 1, last);  // get the subset of test[first + 1, last]
	exist[first] = false;
	Subset(test, exist, first + 1, last);
}

void PrintSubset(int test[], bool exist[], int last)
{
	for(int index = 0; index <= last; index++)
	{
		if(exist[index])  // if  current element is in the subset, print it.
		{
			cout << test[index] << " ";
		}
	}
	cout << endl;
}
