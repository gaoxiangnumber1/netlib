#ifndef NETLIB_NETLIB_BUFFER_H_
#define NETLIB_NETLIB_BUFFER_H_

#include <netlib/copyable.h>

#include <assert.h> // assert()

#include <vector>
#include <string>
#include <algorithm>

namespace netlib
{

class Buffer: public Copyable
{
public:
	static const int kPrepend = 8;
	static const int kInitialSize = 1024;

	Buffer():
		buffer_(kPrepend + kInitialSize),
		read_index_(kPrepend),
		write_index_(kPrepend)
	{
		assert(PrependableByte() == kPrepend);
		assert(ReadableByte() == 0);
		assert(WritableByte() == kInitialSize);
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
	void Append(const char *data, int length)
	{
		EnsureWritableByte(length);
		std::copy(data, data + length, BeginWrite());
		FinishWritten(length);
	}
	void EnsureWritableByte(int length)
	{
		if(WritableByte() < length)
		{
			MakeSpace(length);
		}
		assert(WritableByte() >= length);
	}
	const char *BeginWrite() const
	{
		return Begin() + write_index_;
	}
	char *BeginWrite()
	{
		return Begin() + write_index_;
	}
	void FinishWritten(int length)
	{
		write_index_ += length;
	}

	void RetrieveAll()
	{
		read_index_ = write_index_ = kPrepend;
	}
	const char *Peek() const // The begin pointer of data to be read.
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
	int ReadFd(int fd, int *saved_errno);

private:
	const char *Begin() const
	{
		return &(*buffer_.begin());
	}
	char *Begin()
	{
		return &(*buffer_.begin());
	}
	void MakeSpace(int length)
	{
		// 1. There is really not enough space.
		if(PrependableByte() + WritableByte() < length + kPrepend)
		{
			buffer_.resize(write_index_ + length);
		}
		// 2. Move readable data to the front, make space inside buffer.
		else
		{
			int readable = ReadableByte();
			std::copy(Begin() + read_index_, Begin() + write_index_, Begin() + kPrepend);
			read_index_ = kPrepend;
			write_index_ = read_index_ + readable;
			assert(readable == ReadableByte());
		}
	}

	std::vector<char> buffer_;
	int read_index_;
	int write_index_;
};

}

#endif // NETLIB_NETLIB_BUFFER_H_
