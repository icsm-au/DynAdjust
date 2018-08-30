//============================================================================
// Name         : trace.hpp
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
// Description  : Provides debug trace functionality when MFC is not used
//============================================================================

#ifndef __DEBUG_H__
#define __DEBUG_H__

#if defined(_MSC_VER)
	#if defined(LIST_INCLUDES_ON_BUILD) 
		#pragma message("  " __FILE__) 
	#endif
#endif

#ifdef _DEBUG

void _trace(const char *fmt, ...);

#ifndef ASSERT
#define ASSERT(x) {if(!(x)) _asm{int 0x03}}
#endif	// #ifndef ASSERT

#ifndef VERIFY
#define VERIFY(x) {if(!(x)) _asm{int 0x03}}
#endif	// #ifndef VERIFY

#else

#ifndef ASSERT
#define ASSERT(x)
#endif	// #ifndef ASSERT

#ifndef VERIFY
#define VERIFY(x) x
#endif	// #ifndef VERIFY

#endif	// #ifdef _DEBUG

#ifdef _DEBUG

#ifndef TRACE
#define TRACE _trace
#endif	// #ifndef TRACE

#else

inline void _trace(const char* fmt, ...) { }

#ifndef TRACE
#define TRACE  1 ? (void)0 : _trace
#endif	// #ifndef TRACE

#endif	// #ifdef _DEBUG
#endif // __DEBUG_H__
