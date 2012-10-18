#pragma once

// hommage for boost::optional

template <typename T>
struct optional
{
public:
	optional()
		:
		isInited_(false)
	{
	}
	
	optional(T t)
		:
		value_(t),
		isInited_(true)
	{
	}
	
	optional& operator = (const T& value)
	{
		value_ = value;
		isInited_ = true;
		return *this;
	}

	const T& operator * () const
	{
		return value_;
	}

	T& operator * ()
	{
		return value_;
	}
	
	operator const T* () const
	{
		return isInited_ ? &value_ : (T*)NULL;
	}
	
private:
	T value_;
	bool isInited_;
};


