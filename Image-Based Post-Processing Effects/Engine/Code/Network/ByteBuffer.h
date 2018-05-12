#pragma once
#include <vector>
#include <iostream>
#include <exception>

class ByteBuffer
{
public:
	class UnderflowException : public std::exception
	{
	public:
		virtual char const* what() const override
		{
			return "ByteBuffer Underflow Exception";
		}
	};


	ByteBuffer(const ByteBuffer &other);
	ByteBuffer(size_t &capacity = standardCapacity);
	~ByteBuffer();

	/*Getters*/
	size_t position() const;
	size_t size() const;
	size_t capacity() const;
	//returns amount of bytes remaining, to be read
	inline size_t remaining() const;

	/*Setters*/
	//sets the current position
	void position(const size_t &pos);
	//marks the current position
	void mark();
	//resets the current position to a marked position
	void reset();
	//discards all bytes smaller than position, and reorders data
	void discard();
	//rewinds position to start
	void rewind();
	//moved position to the end, ready to append
	void head();
	//clears the buffers content, keeps the capacity
	void clear();

	const char* content() const;

	template <typename T> T get();
	template <typename T> void get(T *buffer, const size_t size);

	template <typename T> ByteBuffer& operator+=(const T &x);
	template <typename T> void put(const T &x);
	template <typename T> void put(const T *x, const size_t size);
	void put(const ByteBuffer &other);


	ByteBuffer& operator=(const ByteBuffer &other);

	friend std::ostream& operator<<(std::ostream &os, const ByteBuffer &buffer);

private:
	static size_t standardCapacity;
	size_t capacity_;
	size_t pos_;
	size_t size_;
	size_t mark_;
	char *data;

	void ensureSpace(size_t space);
	void grow();
};


size_t ByteBuffer::remaining() const
{
	return size_ - pos_;
}

template<typename T>
T ByteBuffer::get()
{
	if (remaining() < sizeof(T))
	{
		throw UnderflowException();
	}

	T *ptr = (T*)(data + pos_);
	pos_ += sizeof(T);
	return *ptr;
}

template<typename T>
inline void ByteBuffer::get(T *buffer, const size_t size)
{
	const size_t byteCount = sizeof(T) * size;
	if (remaining() < byteCount)
	{
		throw UnderflowException();
	}

	T *ptr = (T*)(data + pos_);

	for (size_t i = 0; i < size; ++i)
	{
		buffer[i] = ptr[i];
	}
	pos_ += byteCount;
}

template<typename T>
inline ByteBuffer& ByteBuffer::operator+=(const T &x)
{
	head();
	put(x);
	return *this;
}

template<typename T>
void ByteBuffer::put(const T &x)
{
	ensureSpace(sizeof(T));
	T *ptr = (T*)(data + pos_);
	*ptr = x;
	pos_ += sizeof(T);

	if (size_ < pos_)
	{
		size_ = pos_;
	}
}

template<typename T>
void ByteBuffer::put(const T *x, const size_t size)
{
	for (size_t i = 0; i<size; ++i)
	{
		put(x[i]);
	}
}


