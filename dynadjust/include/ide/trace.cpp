//============================================================================
// Name         : trace.cpp
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

#ifdef _DEBUG
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>

void _trace(const char *fmt, ...)
{
	char out[1024];
	va_list body;
	va_start(body, fmt);
	vsprintf(out, fmt, body);
	va_end(body);

	// OutputDebugString defined in winbase.h
	OutputDebugString((LPCTSTR)out);
}

#endif
