//============================================================================
// Name         : dnaprocessfuncs.cpp
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
// Description  : Common process and fork functions
//============================================================================

#include <include/functions/dnaprocessfuncs.hpp>

using namespace boost::filesystem;

#if defined(__linux) || defined(sun) || defined(__unix__)
void  parse(char *line, char **argv)
{
	 while (*line != '\0') {       /* if not the end of line ....... */ 
		  while (*line == ' ' || *line == '\t' || *line == '\n')
			   *line++ = '\0';     /* replace white spaces with 0    */
		  *argv++ = line;          /* save the argument position     */
		  while (*line != '\0' && *line != ' ' && 
				 *line != '\t' && *line != '\n') 
			   line++;             /* skip the argument until ...    */
	 }
	 *argv = NULL;                 /* mark the end of argument list  */
}

bool execute(char **argv, char **env)
{
	pid_t const cpid = fork();
	
	if (cpid < 0)
		return false;
	else if (cpid == 0) {
		// Child process
		//int success;
		execvpe(*argv, argv, env);

		return true;
	
	}
	else {
		// Parent process
		int status;
		
		// Wait for child to complete...
		wait(&status);
		
		//return true;

		if(WEXITSTATUS(status) == 0)
		{
			// Program succeeded
			//delme << " Program succeeded" << endl;
			return true;
		}
		else
		{
			// Program failed but exited normally
			//delme << " Program failed but exited normally: " << WEXITSTATUS(status) << endl;
			return false;
		}
	
	}
}
#elif defined(__APPLE__)
void  parse(char *line, char **argv)
{
	 while (*line != '\0') {       /* if not the end of line ....... */
		  while (*line == ' ' || *line == '\t' || *line == '\n')
			   *line++ = '\0';     /* replace white spaces with 0    */
		  *argv++ = line;          /* save the argument position     */
		  while (*line != '\0' && *line != ' ' &&
				 *line != '\t' && *line != '\n')
			   line++;             /* skip the argument until ...    */
	 }
	 *argv = NULL;                 /* mark the end of argument list  */
}

bool execute(char **argv, char **env)
{
	pid_t const cpid = fork();
	int status(0);

	if (cpid < 0)
		return false;
	else if (cpid == 0) {
		// Child process
		//int success;
		execve(*argv, argv, env);

		return true;

	}
	else {
		// Parent process
		int status;

		// Wait for child to complete...
		wait(&status);

		//return true;

		if(WEXITSTATUS(status) == 0)
		{
			// Program succeeded
			//delme << " Program succeeded" << endl;
			return true;
		}
		else
		{
			// Program failed but exited normally
			//delme << " Program failed but exited normally: " << WEXITSTATUS(status) << endl;
			return false;
		}

	}
}

#endif

bool run_command(const string& exec_path_name, bool validateReturnCode)
{	
#if defined(_WIN32) || defined(__WIN32__)
	// Windows
	
	//pid_t cpid = -1;
	
	STARTUPINFO startup;
	PROCESS_INFORMATION process;
	
	memset(&startup, 0, sizeof(STARTUPINFO));
	memset(&process, 0, sizeof(PROCESS_INFORMATION));
	
	startup.cb = sizeof(STARTUPINFO);

	DWORD returnCode;
	if (CreateProcess(0, (LPSTR)exec_path_name.c_str(), 0, 0, FALSE,
		0, 0, 0, &startup, &process))
	{
		WaitForSingleObject(process.hProcess, INFINITE);
		// Capture the return code
		GetExitCodeProcess(process.hProcess, &returnCode);
		CloseHandle(process.hThread);
		CloseHandle(process.hProcess);
		if (validateReturnCode)
		{
			if (returnCode == EXIT_SUCCESS)
				return true;
			else
				return false;
		}
		return true;
	}
	return false;
	
#elif defined(__APPLE__)
	// Apple Mac code here to launch command
	std::system(exec_path_name.c_str());
	return true;
		
#elif defined(__OS2__)
	// OS/2 code here to launch command
	
	throw runtime_error("Function not defined yet.");
	
#elif defined(__linux) || defined(sun) || defined(__unix__)
#define WR_MAX_ARG 100
	// Linux
	std::system(exec_path_name.c_str());
	return true;

#endif
	return true;
}
