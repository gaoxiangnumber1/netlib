#ifndef NETLIB_NETLIB_BUFFER_H_
#define NETLIB_NETLIB_BUFFER_H_

#include <netlib/copyable.h>

#include <assert.h> // assert()

#include <vector>
#include <string>

namespace netlib
{

class Buffer: public Copyable
{
public:
	static const int kPrepend = 8;
	static const int kInitialSize = 1024;

	explicit Buffer(int initial_size = kInitialSize):
		buffer_(kPrepend + initial_size),
		read_index_(kPrepend),
		write_index_(kPrepend)
	{
		assert(PrependableByte() == kPrepend);
		assert(ReadableByte() == 0);
		assert(WritableByte() == initial_size);
	}
	// Default dtor/copy-ctor/assignment-op are okay.

	int PrependableByte() const
	{
		return read_index_;
	}
	int ReadableByte() const
	{
		return write_index_ - read_index_;
	}
	int WritableByte() const
	{
		return static_cast<int>(buffer_.size()) - write_index_;
	}

	const char *FindCRLF(const char *start = BeginRead()) const
	{
		assert(BeginRead() <= start && start <= BeginWrite());
		const char *ret = nullptr, *end = BeginWrite() - 1;
		for(const char *ptr = start; ptr < end; ++ptr)
		{
			if(*ptr == '\r' && *(ptr + 1) == '\n')
			{
				ret = ptr;
				break;
			}
		}
		return ret;
	}

	void Append(const char *data, int length)
	{
		EnsureWritableByte(length);
		Copy(data, length, BeginWrite());
		write_index_ += length;
	}
	// TODO: Is necessary to overload?
	const char *BeginWrite() const
	{
		return Begin() + write_index_;
	}
	char *BeginWrite()
	{
		return Begin() + write_index_;
	}

	void RetrieveAll()
	{
		read_index_ = write_index_ = kPrepend;
	}
	const char *BeginRead() const // The begin pointer of data to be read.
	{
		return Begin() + read_index_;
	}
	void Retrieve(int length)
	{
		assert(length <= ReadableByte());
		read_index_ += length;
	}
	std::string RetrieveAsString()
	{
		std::string str(Peek(), ReadableByte());
		RetrieveAll();
		return str;
	}
	int ReadFd(int fd, int &saved_errno);

private:
	const char *Begin() const
	{
		return &(*buffer_.begin());
	}
	char *Begin()
	{
		return &(*buffer_.begin());
	}
	void EnsureWritableByte(int length)
	{
		if(WritableByte() < length)
		{
			// 1. There is really not enough space.
			if(PrependableByte() + WritableByte() < length + kPrepend)
			{
				buffer_.resize(write_index_ + length);
			}
			// 2. Move readable data to the front, make space inside buffer.
			else
			{
				assert(read_index_ > kPrepend);
				int readable_byte = ReadableByte();
				Copy(BeginRead(), readable_byte, Begin() + kPrepend);
				read_index_ = kPrepend;
				write_index_ = read_index_ + readable_byte;
				assert(readable_byte == ReadableByte());
			}
		}
		assert(WritableByte() >= length);
	}
	void Copy(const char *to_copy, const int length, char *to_write)
	{
		for(int index = 0; index < length; ++index)
		{
			*(to_write++) = *(data + index);
		}
	}

	std::vector<char> buffer_;
	int read_index_;
	int write_index_;
};

}

#endif // NETLIB_NETLIB_BUFFER_H_
