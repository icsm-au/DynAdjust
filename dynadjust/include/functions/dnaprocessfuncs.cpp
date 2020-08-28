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
	
	throw runtime_error("Function not defined yet.");
	
#elif defined(__OS2__)
	// OS/2 code here to launch command
	
	throw runtime_error("Function not defined yet.");
	
#elif defined(__linux) || defined(sun) || defined(__unix__)
#define WR_MAX_ARG 100
	// Linux
	
	/*
	std::system(exec_path_name.c_str());
	// see wifexited and wifesignaled and wifexitstatus
	return true;
	
	// Split the input command up into an array of words stored
	// in a contiguous block of memory. The array contains pointers
	// to each word.
	// Don't forget the terminating `\0' character.
	char const * const c_str = exec_path_name.c_str();
	vector<char> vec(c_str, c_str + exec_path_name.size() + 1);

	// Splitting the command up into an array of words means replacing
	// the whitespace between words with '\0'. Life is complicated
	// however, because words protected by quotes can contain whitespace.
	//
	// The strategy we adopt is:
	// 1. If we're not inside quotes, then replace white space with '\0'.
	// 2. If we are inside quotes, then don't replace the white space
	//    but do remove the quotes themselves. We do this naively by
	//    replacing the quote with '\0' which is fine if quotes
	//    delimit the entire word. However, if quotes do not delimit the
	//    entire word (i.e., open quote is inside word), simply discard
	//    them such as not to break the current word.
	char inside_quote = 0;
	char c_before_open_quote = ' ';
	vector<char>::iterator it = vec.begin();
	vector<char>::iterator itc = vec.begin();
	vector<char>::iterator const end = vec.end();
	for (; it != end; ++it, ++itc) {
		char const c = *it;
		if (!inside_quote) {
			if (c == '\'' || c == '"') {
				if (c_before_open_quote == ' ')
					*itc = '\0';
				else
					--itc;
				inside_quote = c;
			} else {
				if (c == ' ')
					*itc = '\0';
				else
					*itc = c;
				c_before_open_quote = c;
			}
		} else if (c == inside_quote) {
			if (c_before_open_quote == ' ')
				*itc = '\0';
			else
				--itc;
			inside_quote = 0;
		} else
			*itc = c;
	}

	// Clear what remains.
	for (; itc != end; ++itc)
		*itc = '\0';

	// Build an array of pointers to each word.
	it = vec.begin();
	vector<char *> argv;
	char prev = '\0';
	for (; it != end; ++it) {
		if (*it != '\0' && prev == '\0')
			argv.push_back(&*it);
		prev = *it;
	}
	argv.push_back(0);

	pid_t const cpid = ::fork();
	
	if (cpid == 0) {
		// Child
		execvp(argv[0], &*argv.begin());

		// If something goes wrong, we end up here
		cout << "execvp of " << path(exec_path_name).string() << " failed: "
			  << strerror(errno) << endl;
		_exit(1);
		return false;
	}

	*/

	char  line[1024];
	char *argv[64];
	strcpy(line, exec_path_name.c_str());
	parse(line, argv);
	
	char  environment_vars[2048];
	char *env[64];
	sprintf(environment_vars, "%s %s %s", 
		"PATH=/usr/bin/:$PATH",
		"LD_LIBRARY_PATH=/usr/lib64:/usr/lib:$LD_LIBRARY_PATH",
		"MALLOC_CHECK_=0");
	parse(environment_vars, env);
		
	bool b=execute(argv, env);

	//delme.close();
	
	if (!b)
	{
		// If something goes wrong, we end up here
		//cout << "execvp of " << exec_path_name << " failed: " << strerror(errno) << endl;
		return false;
	}

#endif
	return true;
}
