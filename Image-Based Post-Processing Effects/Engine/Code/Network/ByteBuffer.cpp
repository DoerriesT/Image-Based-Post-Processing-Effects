#include "ByteBuffer.h"
#include <stdexcept>
#include "..\Utilities\Utility.h"

size_t ByteBuffer::standardCapacity = 64;

ByteBuffer::ByteBuffer(const ByteBuffer &other) : ByteBuffer()
{
	put(other);
}

ByteBuffer::ByteBuffer(size_t &capacity) : data(new char[capacity]), capacity_(capacity), pos_(0), size_(0), mark_(-1)
{
}

ByteBuffer::~ByteBuffer()
{
	delete[] data;
}

size_t ByteBuffer::position() const
{
	return pos_;
}


void ByteBuffer::position(const size_t &pos)
{
	if (pos < 0 || pos > size_)
	{
		throw std::invalid_argument("ByteBuffer position()");
	}
	pos_ = pos;
}

size_t ByteBuffer::size() const
{
	return size_;
}

size_t ByteBuffer::capacity() const
{
	return capacity_;
}

void ByteBuffer::head()
{
	pos_ = size_;
}

const char* ByteBuffer::content() const
{
	return data;
}

void ByteBuffer::mark()
{
	mark_ = pos_;
}

void ByteBuffer::reset()
{
	if (mark_ < 0)
	{
		throw std::exception("ByteBuffer reset failed, no marked position.");
	}
	pos_ = mark_;
}

void ByteBuffer::rewind()
{
	pos_ = 0;
	mark_ = -1;
}

void ByteBuffer::clear()
{
	pos_ = 0;
	size_ = 0;
	mark_ = -1;
}

void ByteBuffer::discard()
{
	const size_t discardBytes = pos_;

	pos_ = 0;
	size_ -= discardBytes;
	mark_ -= discardBytes;

	for (size_t i = 0; i<size_; ++i)
	{
		data[i] = data[discardBytes + i];
	}
}

void ByteBuffer::grow()
{
	capacity_ = Util::grow(data, capacity_);
}

std::ostream& operator<<(std::ostream &os, const ByteBuffer &buffer)
{
	os << '{';
	if (buffer.size_ > 0)
	{
		for (size_t i = buffer.pos_; i < buffer.size_ - 1; ++i)
		{
			os << (int)buffer.data[i] << ',' << ' ';
		}

		os << (int)buffer.data[buffer.size_ - 1];
	}
	os << '}';

	return os;
}

void ByteBuffer::ensureSpace(size_t space)
{
	while (pos_ + space > capacity_)
	{
		grow();
	}
}

void ByteBuffer::put(const ByteBuffer &other)
{
	put<char>(other.data, other.size_);
}

ByteBuffer& ByteBuffer::operator=(const ByteBuffer &other)
{
	clear();
	put(other);

	return *this;
}
