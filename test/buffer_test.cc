#include <stdio.h>

#include <netlib/buffer.h>

using std::string;
using netlib::Buffer;

int main()
{
	Buffer buffer;
	buffer.Append("gaoxiangnumber1", 15);
	assert(buffer.FindCRLF() == nullptr);
	buffer.Append("helloworld\r\n", 12);
	assert(*buffer.FindCRLF() == '\r');

	buffer.Append(string(2000, 'a'));
	assert(buffer.PrependableByte() == 8 &&
	       buffer.ReadableByte() == 2027 &&
	       buffer.WritableByte() == 0);
	printf("Retrieve 27 bytes:%s", buffer.RetrieveAsString(27).c_str());
	assert(buffer.PrependableByte() == 35 &&
	       buffer.ReadableByte() == 2000 &&
	       buffer.WritableByte() == 0);

	buffer.Append(string(20, 'b'));
	assert(buffer.PrependableByte() == 8 &&
	       buffer.ReadableByte() == 2020 &&
	       buffer.WritableByte() == 7);
	assert(buffer.RetrieveAllAsString() == string(2000, 'a') + string(20, 'b'));
	printf("All passed!\n");
}
