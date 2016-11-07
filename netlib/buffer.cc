#include <netlib/buffer.h>

#include <sys/uio.h> // readv()
#include <errno.h> // errno

using netlib::Buffer;

int Buffer::ReadFd(int fd, int *saved_errno)
{
	char extra_buffer[65536];
	const int writable = WritableByte();
	// 	struct iovec
	// 	{
	// 		void *iov_base; // Starting address.
	// 		size_t iov_len; // Number of bytes to transfer.
	// 	};
	struct iovec vec[2];
	vec[0].iov_base = Begin() + write_index_;
	vec[0].iov_len = writable;
	vec[1].iov_base = extra_buffer;
	vec[1].iov_len = sizeof extra_buffer;
	// ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
	// iov points to an array of iovec structures:
	// readv() reads `iovcnt` buffers from the file associated with the file descriptor fd
	// into the buffers described by iov("scatter input"). It works just like read(2) except
	// that multiple buffers are filled.
	// Buffers are processed in array order: readv() completely fills iov[0] before
	// proceeding to iov[1], and so on.
	// The data transfer performed by readv() is atomic: readv() is guaranteed to read
	// a contiguous block of data from the file, regardless of read operations performed
	// in other threads or processes that have file descriptors referring to the same open
	// file description (see open(2)).
	// Return the number of bytes read on success; -1 is returned on error, and errno is set.
	const int read_byte = static_cast<int>(readv(fd, vec, 2));
	if(read_byte < 0)
	{
		*saved_errno = errno;
	}
	else if(read_byte <= writable)
	{
		write_index_ += read_byte;
	}
	else
	{
		write_index_ = static_cast<int>(buffer_.size());
		Append(extra_buffer, read_byte - writable);
	}
	return read_byte;
}
