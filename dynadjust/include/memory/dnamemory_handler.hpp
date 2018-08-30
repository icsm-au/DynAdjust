//============================================================================
// Name         : dnamemory_handler.hpp
// Author       : Roger Fraser
// Contributors :
// Version      : 1.00
// Copyright    : Copyright 2017 Geoscience Australia
//
//                Licensed under the Apache License, Version 2.0 (the "License");
//                you may not use this file except in compliance with the License.
//                You may obtain a copy of the License at
//               
//                http ://www.apache.org/licenses/LICENSE-2.0
//               
//                Unless required by applicable law or agreed to in writing, software
//                distributed under the License is distributed on an "AS IS" BASIS,
//                WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//                See the License for the specific language governing permissions and
//                limitations under the License.
//
// Description  : Interface for DynAdjust matrix memory handler
//============================================================================

#ifndef DNAMEMORY_H_
#define DNAMEMORY_H_

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

//const UINT32 KILOBYTE_SIZE = 1024;				// 2 ^ 10
//const UINT32 MEGABYTE_SIZE = 1048576;				// 2 ^ 20
//const UINT32 GIGABYTE_SIZE = 1073741824;			// 2 ^ 30

typedef enum _MEM_UNIT_ {
	KILOBYTE_SIZE = 1024,
	MEGABYTE_SIZE = 1048576,
	GIGABYTE_SIZE = 1073741824
} MEM_UNIT;

// The maximum range of a long integer variable in C++ is 2 ^ 32, so
// the following will be truncated
//const UINT32 TERABYTE_SIZE = 1099511627776;		// 2 ^ 40

using namespace std;

namespace dynadjust { namespace memory {

class new_handler_holder 
{
public:
	explicit new_handler_holder(new_handler nh)					// acquire current
		: _handler(nh) {}										// new handler
	~new_handler_holder() { set_new_handler(_handler); }		// release it
private:
	new_handler _handler;
	new_handler_holder(const new_handler_holder&);				// prevent copying
	new_handler_holder& operator=(const new_handler_holder&);	//   ''      ''
};

template<typename T>
class new_handler_support {
public:
	static new_handler set_new_handler(new_handler p) noexcept;
	static void* operator new(size_t size) noexcept(false);

private:
	static new_handler _current_handler;
};

template<typename T>
new_handler new_handler_support<T>::set_new_handler(new_handler p) noexcept
{
	new_handler old_handler = _current_handler;
	_current_handler = p;
	return old_handler;
}

template<typename T>
void* new_handler_support<T>::operator new(size_t size) noexcept(false)
{
	new_handler_holder h(set_new_handler(_current_handler));
	return ::operator new(size);
}

// this initialises each _current_handler to null
template<typename T>
new_handler new_handler_support<T>::_current_handler = 0;
	
}	// namespace memory 
}	// namespace dynadjust 

#endif	// DNAMEMORY_H_

