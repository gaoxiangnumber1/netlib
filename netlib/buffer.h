#ifndef NETLIB_NETLIB_BUFFER_H_
#define NETLIB_NETLIB_BUFFER_H_

#include <assert.h> // assert()

#include <vector>
#include <string>

#include <netlib/copyable.h>

namespace netlib
{

// Review:	ReadFd#errno, Append(const char*, int)#w_i_, EnsureWritableByte
//					Retrieve, RetrieveAll, RetrieveUntil#assert, Prepend, PeekInt32#assert

// Interface:
// Ctor -> +PrependableByte -> +ReadableByte -> +WritableByte
// Getter: PrependableByte, ReadableByte, WritableByte
// FindCRLF: (const char*), () -> +ReadableBegin -> -WritableBegin
//			+ReadableBegin -> -BufferBegin
//			-WritableBegin -> -BufferBegin
// ReadFd -> +Append(const char*, int)
// Append(const string&) -> Append(const char*, int) -> -EnsureWritableByte -> -Copy
//			-EnsureWritableByte -> -Copy
// Prepend
// RetrieveAllAsString -> RetrieveAsString -> Retrieve -> RetrieveAll
// RetrieveUntil -> +ReadableBegin -> -WritableBegin -> +Retrieve
// PeekInt32

class Buffer: public Copyable
{
public:
	static const int kPrepend = 8;
	static const int kOneKilobyte = 1024;

	explicit Buffer(int initial_size = kOneKilobyte);
	// Default dtor/copy-ctor/assignment-op are okay.

	// Getter.
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

	const char *FindCRLF(const char *start) const;
	const char *FindCRLF() const;
	// The begin pointer of data to be read. Must be read-only.
	const char *ReadableBegin() const
	{
		return BufferBegin() + read_index_;
	}

	// Input API: Read from socket and store the data in buffer.
	int ReadFd(int fd, int &saved_errno);
	void Append(const std::string &data);
	void Append(const char *data, int length);
	void Prepend(const void *data, int length);

	// Output API:
	std::string RetrieveAllAsString();
	std::string RetrieveAsString(int length);
	void Retrieve(int length);
	void RetrieveUntil(const char *until);
	void RetrieveAll();

	int32_t PeekInt32();

private:
	const char *BufferBegin() const
	{
		return &(*buffer_.begin());
	}
	char *BufferBegin()
	{
		return &(*buffer_.begin());
	}
	const char *WritableBegin() const
	{
		return BufferBegin() + write_index_;
	}
	char *WritableBegin()
	{
		return BufferBegin() + write_index_;
	}

	void EnsureWritableByte(int length);
	void Copy(const char *to_copy, const int length, char *to_write);

	std::vector<char> buffer_;
	int read_index_;
	int write_index_;
};

}

#endif // NETLIB_NETLIB_BUFFER_H_
