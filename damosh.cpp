#include <cstdio>
#include <readline/history.h>
#include <readline/readline.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <signal.h>
#include "damoshparser.h"
using namespace std;

extern int errno;
extern char** environ;
bool interactive = false;
int waitingpid = 0;
bool color_output = false;
string msg_head = "damosh[id_null][dir_null]usr_null ";

#ifdef DEBUG
void debug(string msg)
{
    cerr << msg_head << "{" << getpid() << "}" << msg << endl;
}
#else
void debug(string msg) {}
#endif

void trim(string& str)
{
    string::size_type pos = str.find_last_not_of(' ');
    if (pos != string::npos)
    {
        str.erase(pos + 1);
        pos = str.find_first_not_of(' ');
        if (pos != string::npos) str.erase(0, pos);
    }
    else str.erase(str.begin(), str.end());
}

string expandHomeDirectory(string str)
{
    if (str[0] == '/')
        return str;
    if (str[0] == '~')
    {
        if (str.size() == 1)
        {
            string home_dir = string(getenv("HOME"));
            return home_dir;
        }
        else if (str[1] == '/')
        {
            string home_dir = string(getenv("HOME"));
            return home_dir + str.substr(1);
        }
        else
        {
            int pos = str.find("/");
            string username = str.substr(1, pos - 1);
			struct passwd *record = getpwnam(username.c_str());
			string path;
			if (record && record->pw_dir)
				path = string(record->pw_dir) + str.substr(pos);
			else
				path = str;
			return path;
		}
	}
	
	return str;
}

void execute(DAMOSH_COMMAND_PARSER_PART r)
{
	char **arg_list = (char**)malloc(sizeof(char*) * (r.arguments.size() + 1));
	char *searchPath = getenv("MYPATH");
	if (searchPath)
	{
		int len = strlen(searchPath);
		for(int i=0;i<len;i++)
			if (searchPath[i] == '$')
				searchPath[i] = ':';
	}
	if (!searchPath)
	{
		searchPath = getenv("PATH");
	}
	
	if (!searchPath)
	{
		searchPath = (char*)malloc(sizeof(char) * 1);
		strcpy(searchPath, "");
	}

	string path = r.getAbsolutePath(string(searchPath));
	if (!searchPath)
		free(searchPath);
	
	arg_list[0] = (char*)malloc(path.length() + 1);
	strcpy(arg_list[0], path.c_str());
	arg_list[0][path.length()] = NULL;
	int i;
	for(i = 0;i < r.arguments.size();i++)
	{
		arg_list[i+1] = (char*)malloc(r.arguments[i].length() + 1);
		strcpy(arg_list[i+1], r.arguments[i].c_str());
		arg_list[i+1][r.arguments[i].length()] = NULL;
	}
	arg_list[r.arguments.size() + 1] = NULL;
	
	FILE* fp;
	int fd;
	
	if (r.redirIn != "")
	{
		fp = fopen(r.redirIn.c_str(), "r");
		fd = fileno(fp);
		if (dup2(fd, STDIN_FILENO) == -1)
			cerr << msg_head << "Unable To Redirect STDIN From " << r.redirIn << endl;
	}

	if (r.redirOut != "")
	{
		fp = fopen(r.redirOut.c_str(), "w");
		fd = fileno(fp);
		if (dup2(fd, STDOUT_FILENO) == -1)
			cerr << msg_head << "Unable To Redirect STDOUT To " << r.redirOut << endl;
	}

	if (r.redirAppend != "")
	{
		fp = fopen(r.redirAppend.c_str(), "a");
		fd = fileno(fp);
		if (dup2(fd, STDOUT_FILENO) == -1)
			cerr << msg_head << "Unable To Redirect and Append STDOUT To " << r.redirAppend << endl;
	}
	execv(arg_list[0], arg_list);
	cout << msg_head << "Unable To Execute Command [ERROR: " << errno << "]" << endl;
	exit(-1);
	for(i = 0;i < r.arguments.size();i++)
		free(arg_list[i]);
	free(arg_list);
	return;
}

void spawn(DAMOSH_COMMAND_PARSER_RESULT_PART r)
{
	int pid = fork();
	
	if (pid == 0)
		execute(r);
	else if (pid > 0)
	{
		if (!r.background)
		{
			waitingpid = pid;
			waitpid(pid, NULL, NULL);
		}
	}
	else
		cout << msg_head << "ERROR! Fork Failed" << endl;
	
	return;
}

//jdisosbisbsiwn2jowjwh9shsisjw9whw9wnwiwnwnwjwownwowjw9whw9wjw9wjw9wj392n39wnw93h93h393h3i3j3i3o3n39wjs8deij3ie9e]3ncijdjdidnjeienekenejeonejeoenekekwkwkkwkwkskskskekoeowjwnodhrjebeieneneijenejejekeoejienekeieg83h3ie8d9j3if8ru}hri84hrjirirhekdh93nekieuebe
int spawn_for_pipe(vector<DAMOSH_COMMAND_PARSER_RESULT_PART>::iterator begin, vector<DAMOSH_COMMAND_PARSER_RESULT_PART>::iterator end)
{
	int fds[2];
	if (pipe(fds) == -1)
		cout << msg_head << "Unable To Create Pipe [ERROR: " << errno << "]" << endl;
	vector<DAMOSH_COMMAND_PARSER_RESULT_PART>::iterator nxtItr = begin + 1;
	int pid = fork();
	if (pid == 0)
	{
		if (begin->pipe_with_next)
		{
			if (dup2(fds[1], STDOUT_FILENO) == -1)
				cerr << msg_head << "dup2 Failed. (STDOUT)" << endl;
		}

		if (close(fds[0]) == -1)
			cerr << msg_head << "Close Failed. (PIPE_READ)" << endl;

		if (close(fds[1]) == -1)
			cerr << msg_head << "Close Failed. (PIPE_WRITE)" << endl;
		execute(*begin);
	}
	else if (pid > 0)
	{
		if (dup2(fds[0], STDIN_FILENO) == -1)
			cerr << msg_head << "dup2 Failed. (STDIN)" << endl;
		if (close(fds[0]) == -1)
			cerr << msg_head << "Close Failed. (PIPE_READ)" << endl;
		if (close(fds[1]) == -1)
			cerr << msg_head << "Close Failed. (PIPE_WRITE)" << endl;
		if (nxtItr == end)
		{
			if (!begin->background)
				waitpid(pid, NULL, NULL);
				
			return pid;
		}
		else
			return spawn_for_pipe(nxtItr, end);
	}
	else
		cerr << msg_head << "ERROR! Fork Failed" << endl;
	
	return -1;
}

void sigint_handler(int sig)
{
	if (!interactive)
	{
		cout << endl << msg_head << "KeyboardInterrupt" << endl;
		kill(waitingpid, SIGINT);
		cout << endl;
	}
	else
	{
		cout << endl << msg_head << "KeyboardInterrupt" << endl;
	}
}

int main()
{
	if (isatty(STDOUT_FILENO) && strcmp(getenv("TERM"), "dumb"))
		color_output = true;
	
	if (!color_output)
	{
		cout << "Welcome To Dam.OS Shell[damosh] v0.6" << endl;
		cout << "Licensed Under GPL License." << endl;
		cout << "Initializing..." << endl;
	}
	else
	{
		cout << "\033[1;37mWelcome To \033[1;31mDam.OS Shell\033[m[\033[1;31mdamosh\033[m] \033[1;37mv0.6\033[m" << endl;
		cout << "Licensed Under GPL-3 License." << endl;
		cout << "\033[1;33mInitializing...\033[m" << endl;
		msg_head = "\033[1;31mdamosh[id_null][dir_null]usr_null\033[m ";
	}
	vector<string> history;
	string command;
	DAMOSH_COMMAND_PARSER parser;
	char *input;
	using_history();
	char hostname[1024] = {0};
	gethostname(hostname, 1023);
	string home_dir = string(getenv("HOME"));
	signal(SIGINT, sigint_handler);
    
	while(true)
	{
		interactive = true;
		ostringstream oss;
		char *path_c = (char*)malloc(sizeof(char) * 1024);
		size_t size = 1024;
		getcwd(path_c, size);
		if (!path_c)
		{
			cout << msg_head << "ERROR! Unable To Construct Prompt. [ERROR: " << errno << "]" << endl;
			exit(-1);
		}
		
		string path = string(path_c);
		free(path_c);
		int pos = path.find(home_dir);
		if (pos != string::npos)
			path = "~" + path.substr(home_dir.length(), path.length() - home_dir.length());
		if (color_output)
			oss << "[\033[33m" << getenv("USER") << "\033[m@\033[32m" << hostname << "\033[m] " << path << "> ";
		else
			oss << getenv("USER") << "@" << hostname << " " << path << "> ";
		
		input = readline(oss.str().c_str());
		command = string(input, strlen(input));
		trim(command);
		free(input);
		if (command == "") continue;
		add_history(command.c_str());
		interactive = false;
		
		DAMOSH_COMMAND_PARSER_RESULT r = parser.parse(command);
		
		if (r.parts.size() == 0)
			continue;
		
		if (r.parts[0].command == "quit" || r.parts[0].command == "exit")
			break;
		else if (r.parts[0].command == "history")
		{
			for(int i=0;i<history_length;i++)
			{
				HIST_ENTRY *entry = history_get(i);
				printf("%5d  %s\n", i, entry->line);
			}
		}
		else if (r.parts[0].command == "cd")
		{
			if (r.parts[0].arguments.size() == 0)
				chdir(home_dir.c_str());
			else
			{
				chdir(expandHomeDirectory(r.parts[0].arguments[0]).c_str());
			}
		}
		else if (r.parts[0].command == "setenv")
		{
			if (r.parts[0].arguments.size() == 1)
				unsetenv(r.parts[0].arguments[0].c_str());
			else if (r.parts[0].arguments.size() == 2)
				setenv(r.parts[0].arguments[0].c_str(), r.parts[0].arguments[1].c_str(), true);
			else
			{
				cout << msg_head << "setenv requires 1 or 2 arguments, you give me " << r.parts[0].arguments.size() << " arguments." << endl;
				cout << msg_head << "Please check \"help\" command." << endl;
			}
		}
		else if (r.parts[0].command == "listenv")
		{
			char **ptr = environ;
			while(*ptr != NULL)
			{
				cout << *ptr << endl;
				ptr++;
			}
		}
		else if (r.parts[0].command == "help")
		{
			if (color_output)
			{
				cout << "\033[1;37mDam.OS Shell[damosh] v0.6 -Help-\033[m" << endl << endl;
				printf("%10s %s\n", "\033[1;32msetenv\033[m", "(name) (value) Set environment variables, leave value empty to unset.");
				printf("%10s %s\n", "\033[1;32mlistenv\033[m", "List all environment variables");
				printf("%10s %s\n", "\033[1;32mhelp\033[m", "Display this help");
				printf("%10s %s\n", "\033[1;32mquit\033[m", "Leave this shell");
				cout << endl;
			}
			else
			{
				cout << "Dam.OS Shell[damosh] v0.6 -Help-" << endl << endl;
				printf("%10s %s\n", "setenv", "(name) (value) Set environment variables, leave value empty to unset.");
				printf("%10s %s\n", "listenv", "List all environment variables");
				printf("%10s %s\n", "help", "Display this help");
				printf("%10s %s\n", "quit", "Leave this shell");
				cout << endl;				
			}
		}
		else
		{
			if (r.parts.size() == 1)
				spawn(r.parts[0]);
			else
			{
				int pid = fork();
				if (pid == 0)
				{
					int lastpid = spawn_for_pipe(r.parts.begin(), r.parts.end());
					waitpid(lastpid, NULL, NULL);
					exit(0);
				}
				else
				{
					waitingpid = pid;
					waitpid(pid, NULL, NULL);
				}
			}
		}
	}
}