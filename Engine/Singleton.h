#pragma once
#include <assert.h>

class WindowsManager;

// This implementation heavily inspired by OGRE

template <typename T>
class Singleton
{
private:

	Singleton(const Singleton<T> &);

	// This is an illegal operation
	Singleton& operator=(const Singleton<T> &);

protected:
	static T* msSingleton;

public:

	Singleton(void)
	{
		assert(!msSingleton);
		msSingleton = static_cast<T*>(this);
	}

	~Singleton(void)
	{
		assert(msSingleton);
		msSingleton = 0;
	}

	static T& GetSingleton()
	{
		assert(msSingleton);
		return msSingleton*;
	}

	static T* GetSingletonPtr()
	{
		return msSingleton;
	}
};

