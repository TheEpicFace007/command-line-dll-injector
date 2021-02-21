#include <Util.h>

template<typename T>
bool vector_include(std::vector<T> vec, T content)
{
	BOOST_FOREACH(T elem, vec)
	{
		if (elem == content)
			return true;
	}

	return false;
}