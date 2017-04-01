/*
Compare two strings:
1.	Empty string's size is 0. All empty strings are equal. All empty strings are smaller
	than nonempty strings.
2.	If two strings' common length part are same, then they are equal if they have the same
	length, otherwise, the shorter string is smaller.
3.	Other conditions: Judge according to the first different letter.
*/

// Return: -2 on input error; -1 if string1 < string2;
// 0 if string1 = string2; 1 if string1 >string2
int CompareString(string string1, string string2)
{
	int length1 = string1.size(), length2 = string2.size();

	// Input error:
	if(length1 < 0 || length2 < 0)
	{
		return -2;
	}

	// If there exist at least one empty string
	if(length1 == 0 || length2 == 0)
	{
		if(length1 == 0 && length2 == 0)  // Two empty strings are equal
		{
			return 0;
		}
		else if(length1 != 0)  //string2 is empty, so string1 > string2
		{
			return 1;
		}
		else  // string1 is empty, so string1 < string2
		{
			return -1;
		}
	}

	// Up to now, both strings are not empty.
	int common_length = (length1 < length2 ? length1 : length2);
	for(int index = 0; index < common_length; ++index)
	{
		if(string1[index] < string2[index])
		{
			return -1;
		}
		else if(string1[index] > string2[index])
		{
			return 1;
		}
	}

	// Common part are the same.
	if(length1 == length2)
	{
		return 0;
	}
	else if(length1 > length2)  // string1 is longer, so string1 > string2
	{
		return 1;
	}
	else  // string2 is longer, so string1 < string2
	{
		return -1;
	}
}

