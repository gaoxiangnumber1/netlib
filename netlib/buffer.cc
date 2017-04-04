#include <netlib/buffer.h>

#include <sys/uio.h> // readv()
#include <errno.h> // errno

using netlib::Buffer;
using std::string;

Buffer::Buffer(int initial_size):
	buffer_(kInitialPrependableByte + initial_size),
	read_index_(kInitialPrependableByte),
	write_index_(kInitialPrependableByte)
{
	assert(PrependableByte() == kInitialPrependableByte);
	assert(ReadableByte() == 0);
	assert(WritableByte() == initial_size);
}

const char *Buffer::FindCRLF(const char *start) const
{
	assert(ReadableBegin() <= start && start <= WritableBegin());
	for(const char *ptr = start, *end = WritableBegin() - 1; ptr < end; ++ptr)
	{
		if(*ptr == '\r' && *(ptr + 1) == '\n')
		{
			return ptr;
		}
	}
	return nullptr;
}
const char *Buffer::FindCRLF() const
{
	return FindCRLF(ReadableBegin());
}

int Buffer::ReadFd(int fd, int &saved_errno)
{
	char stack_buffer[64 * kOneKilobyte];
	int writable_byte = WritableByte();
	struct iovec vec[2];
	vec[0].iov_base = WritableBegin();
	vec[0].iov_len = writable_byte;
	vec[1].iov_base = stack_buffer;
	vec[1].iov_len = sizeof stack_buffer;
	int read_byte = static_cast<int>(::readv(fd, vec, 2));
	if(read_byte < 0)
	{
		saved_errno = errno;
	}
	else if(read_byte <= writable_byte)
	{
		write_index_ += read_byte;
	}
	else
	{
		write_index_ += writable_byte;
		Append(stack_buffer, read_byte - writable_byte);
	}
	return read_byte;
}
void Buffer::Append(const string &data)
{
	Append(data.c_str(), static_cast<int>(data.size()));
}
void Buffer::Append(const char *data, int length)
{
	EnsureWritableByte(length);
	MemoryCopy(WritableBegin(), data, length);
	write_index_ += length;
}
void Buffer::EnsureWritableByte(int length)
{
	if(WritableByte() < length)
	{
		// 1. Really not enough space.
		if(PrependableByte() + WritableByte() < kInitialPrependableByte + length)
		{
			buffer_.resize(write_index_ + length);
		}
		// 2. Move readable data to the front, make space inside buffer.
		else
		{
			int readable_byte = ReadableByte();
			MemoryCopy(BufferBegin() + kInitialPrependableByte, ReadableBegin(), readable_byte);
			read_index_ = kInitialPrependableByte;
			write_index_ = read_index_ + readable_byte;
		}
		assert(WritableByte() >= length);
	}
}
void *Buffer::MemoryCopy(void *dest, const void *src, size_t length)
{
	if(dest == nullptr || src == nullptr || dest == src)
	{
		return dest;
	}

	char *dest_ptr = static_cast<char*>(dest);
	const char *src_ptr = static_cast<const char*>(src);
	if(dest_ptr < src_ptr || dest_ptr >= src_ptr + length)
	{
		for(size_t index = 0; index < length; ++index)
		{
			dest_ptr[index] = src_ptr[index];
		}
	}
	else // src_ptr < dest_ptr < src_ptr + length
	{
		for(size_t index = 0; index < length; ++index)
		{
			dest_ptr[length - 1 - index] = src_ptr[length - 1 - index];
		}
	}
	return dest;
}

void Buffer::Prepend(const void *data, int length)
{
	assert(length <= PrependableByte());
	read_index_ -= length;
	const char *to_copy = static_cast<const char*>(data);
	MemoryCopy(BufferBegin() + read_index_, to_copy, length);
}

string Buffer::RetrieveAllAsString()
{
	return RetrieveAsString(ReadableByte());
}
string Buffer::RetrieveAsString(int length)
{
	assert(length <= ReadableByte());
	string result(ReadableBegin(), length);
	Retrieve(length);
	return result;
}
void Buffer::Retrieve(int length)
{
	assert(0 <= length && length <= ReadableByte());
	if(length < ReadableByte())
	{
		read_index_ += length;
	}
	else
	{
		RetrieveAll();
	}
}
void Buffer::RetrieveUntil(const char *until)
{
	assert(ReadableBegin() <= until && until <= WritableBegin());
	Retrieve(static_cast<int>(until - ReadableBegin()));
}
void Buffer::RetrieveAll()
{
	read_index_ = write_index_ = kInitialPrependableByte;
}

int32_t Buffer::PeekInt32()
{
	assert(ReadableByte() >= static_cast<int>(sizeof(int32_t)));
	int32_t be32 = 0;
	MemoryCopy(&be32, ReadableBegin(), sizeof be32);
	return ::be32toh(be32);
}
