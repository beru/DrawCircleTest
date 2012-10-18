#pragma once

#include "arrayutil.h"

template <typename T>
class varray
{
public:
	varray()
		:
		buff_(0),
		maxLength_(0),
		length_(0)
	{
	}
	
	varray(T* buff, size_t maxLength)
		:
		buff_(buff),
		maxLength_(maxLength),
		length_(0)
	{
	}
	
	T* buff_;
	size_t maxLength_;
	size_t length_;
	
	T& operator[] (int i) { return buff_[i]; }
    const T& operator[] (int i) const { return buff_[i]; }
	
	void push_back(const T& val)
	{
		assert(length_ < maxLength_);
		buff_[length_] = val;
		++length_;
	}

};

template <typename T, size_t N>
varray<T> varray_build( T (&arr)[N] )
{
	return varray<T>(arr, N);
}

