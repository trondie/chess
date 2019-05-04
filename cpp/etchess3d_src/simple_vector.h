#ifndef __SIMPLE_VECTOR_H__
#define __SIMPLE_VECTOR_H__

#include <stdlib.h>


// Basic implementation of a vector class
template <class T>
class vector
{
private:
	T *array;
	unsigned int array_size;
	unsigned int alloc_size;

public:
	vector()
	{
		array = 0;
		array_size = 0;
		alloc_size = 0;
	}
	~vector()
	{
		free(array);
	}
	void push_back(T value)
	{
		if (array_size == alloc_size)
		{
			if (alloc_size == 0)
			{
				alloc_size = 1024;
			}
			else
			{
				alloc_size *= 2;
			}

			array = (T *)realloc(array, sizeof(T) * alloc_size);
		}

		if (array)
		{
			array[array_size++] = value;
		}
	}
	void clear()
	{
		if (array)
		{
			free(array);
			array_size = 0;
		}
	}
	unsigned int size()
	{
		return array_size;
	}
	T &operator [](int pos)
	{
		return array[pos];
	}
};


#endif /* __SIMPLE_VECTOR_H__ */