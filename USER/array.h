/**
******************************************************************************
* @file    ringque.h
* @author  Chenxx
* @version V1.0
* @date    2017-3-3
* @brief   This file defines the abridged version of array in cpp STL.
******************************************************************************/
//release information:
#ifndef ARRAY_H
#define ARRAY_H
template <typename T, size_t N>
struct array
{
/**
	* @brief  return size of the array
	* @retval 	size of array
	*/		
	size_t size()
	{
		return N;
	}

/**
	* @brief  Returns a reference to the element at specified location pos. 
	* 				No bounds checking is performed.
	* @param 	pos: position of the element to return
	* @retval Reference to the requested element.
	*/	
	T& operator[] (size_t pos)
	{
		return static_array_[pos];
	}

/**
	* @brief  Returns a reference to the element at specified location pos. 
	* 				No bounds checking is performed.
	* @param 	pos: position of the element to return
	* @retval Reference to the requested element.
	*/
	const T& operator[] (size_t pos) const
	{
		return static_array_[pos];
	}

/**
	* @brief  Returns a reference to the element at specified location pos,
	* 				with bounds checking.
	* @param 	pos: position of the element to return
	* @retval Reference to the requested element.
	*/	
	T& at(size_t pos)
	{
		if(!(pos < N))
		{
#ifdef __EXCEPTIONS
			throw static_array_[0];
#else
			while(1);
#endif		
		}
		return static_array_[pos];
	}
	
/**
	* @brief  Returns a reference to the element at specified location pos,
	* 				with bounds checking.
	* @param 	pos: position of the element to return
	* @retval Reference to the requested element.
	*/		
	const T& at(size_t pos) const
	{
		if(!(pos < N))
		{
#ifdef __EXCEPTIONS
			throw static_array_[0];
#else
			while(1);
#endif		
		}
		return static_array_[pos];
	}
	
	T* begin()
	{
		return static_array_;
	}
	
/**
	* @brief  Returns an iterator to the element following the last element of the container.
	* 				This element acts as a placeholder; 
	*					attempting to access it results in undefined behavior.
	* @param 	pos: position of the element to return
	* @retval Reference to the requested element.
	*/	
	T* end()
	{
		return &static_array_[N];
	}
	
	T static_array_[N == 0 ? 1 : N];
};

#endif
//end of file
