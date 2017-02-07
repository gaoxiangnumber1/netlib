#include <netlib/buffer.h>

#include <sys/uio.h> // readv()
#include <errno.h> // errno

using std::string;
using netlib::Buffer;

Buffer::Buffer(int initial_size):
	buffer_(kPrepend + initial_size),
	read_index_(kPrepend),
	write_index_(kPrepend)
{
	assert(PrependableByte() == kPrepend);
	assert(ReadableByte() == 0);
	assert(WritableByte() == initial_size);
}

const char *Buffer::FindCRLF(const char *start) const
{
	assert(ReadableBegin() <= start && start <= WritableBegin());
	for(const char *ptr = start, *end = WritableBegin() - 2; ptr <= end; ++ptr)
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
	char extra_buffer[65536]; // 2^16B = 2^6KB = 64KB
	int writable_byte = WritableByte();
	// 	struct iovec
	// 	{
	// 		void *iov_base;	// Starting address.
	// 		size_t iov_len;	// Number of bytes to transfer.
	// 	};
	struct iovec vec[2];
	vec[0].iov_base = WritableBegin();
	vec[0].iov_len = writable_byte;
	vec[1].iov_base = extra_buffer;
	vec[1].iov_len = sizeof extra_buffer;
	// ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
	// iov points to an array of iovec structures:
	// readv() reads `iovcnt` buffers from the file associated with the file descriptor fd
	// into the buffers described by iov("scatter input"). It works just like read(2) except
	// that multiple buffers are filled.
	// Buffers are processed in array order: readv() completely fills iov[0] before
	// proceeding to iov[1], and so on.
	// The data transfer performed by readv() is atomic.
	// Return the number of bytes read on success; -1 is returned on error, and errno is set.
	int read_byte = static_cast<int>(::readv(fd, vec, 2));
	if(read_byte <= 0)
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
		Append(extra_buffer, read_byte - writable_byte);
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
	Copy(data, length, WritableBegin());
	write_index_ += length;
}
void Buffer::EnsureWritableByte(int length)
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
			int readable_byte = ReadableByte();
			Copy(ReadableBegin(), readable_byte, BufferBegin() + kPrepend);
			read_index_ = kPrepend;
			write_index_ = read_index_ + readable_byte;
		}
	}
	assert(WritableByte() >= length);
}
void Buffer::Copy(const char *to_copy, const int length, char *to_write)
{
	for(int index = 0; index < length; ++index)
	{
		*(to_write++) = *(to_copy + index);
	}
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
void Buffer::RetrieveAll()
{
	read_index_ = write_index_ = kPrepend;
}
