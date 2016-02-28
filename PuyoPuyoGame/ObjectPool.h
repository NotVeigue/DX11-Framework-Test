#pragma once
#include <stdlib.h>
#include <new>

template <typename T>
class ObjectPool
{
private:
	unsigned int m_poolSize;
	T* m_poolStart;
	T* m_poolTail;

	void push_back(T* object)
	{
		*reinterpret_cast<T**>(object) = m_poolTail;
		m_poolTail = object;
	}

public:
	explicit ObjectPool(unsigned int poolSize)
		: m_poolSize(poolSize)
	{
		// Obtain a chunk of memory equal to the number of objects we were initialized to hold.
		m_poolStart = reinterpret_cast<T*>(malloc(m_poolSize * sizeof(T)));
		m_poolTail = nullptr;

		// Divide the memory chunk into object-sized pieces, each containing a reference to 
		for (unsigned int i = 0; i < m_poolSize; i++)
		{
			push_back(m_poolStart + i);
		}
	}

	~ObjectPool()
	{
		free(m_poolStart);
	}

	T* AllocObject()
	{
		// If we are out of available memory, just return a nullptr.
		// This class is not built for dynamic expansion...
		if (m_poolTail == m_poolStart)
			return nullptr;

		T* tail = m_poolTail;
		m_poolTail = *reinterpret_cast<T**>(m_poolTail);

		// Use placement new to initialize the object
		return new(tail)T();
	}

	void FreeObject(T* object)
	{
		// Call the object's destructor before pushing it back into the list.
		object->~T();
		push_back(object);
	}
};