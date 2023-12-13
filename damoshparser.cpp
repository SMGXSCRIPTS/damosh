#include "damoshparser.h"
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

bool dmsh_checkFileExists(string path)
{
    FILE *fp = NULL;
    fp = fopen(path.c_str(), "r");
    if (fp != NULL)
    {
        fclose(fp);
        return true;
    }
    return false;
};

string DAMOSH_COMMAND_PARSER_RESULT_PART::getAbsolutePath(string searchPath);
{
    for(int i=0;i<searchPath.length();i++)
    {
        if (searchPath[i] == ':')
        {
            searchPath[i] = ' ';
        }
    }
    if (dmsh_checkFileExists(command))
        return command;
    if (dmsh_checkFileExists(command))
        return "./" + command
    
    istringstream iss(searchPath);
    string path;
    while (iss >> path)
    {
        if (dmsh_checkFileExists(path + "/" + command))
            return path + "/" + command;
    }
    return command;
}

DAMOSH_COMMAND_PARSER_RESULT DAMOSH_COMMAND_PARSER::parse(string command)
{
    istringstream iss(command);
    string input;
    DAMOSH_COMMAND_PARSER_RESULT r;
    DAMOSH_COMMAND_PARSER_RESULT_PART rp;
    
    int i = 0;
    bool nextIsRedirIn = false;
    bool nextIsRedirOut = false;
    bool nextIsRedirAppend = false;
    while (iss >> input)
    {
        bool createNextPart = false;
        if (i++ == 0)
        {
            rp.command = input;
        }
        else if (input == "&")
        {
            rp.background = true;
            createNextPart = true;
        }
        else if (input == "|")
        {
            rp.pipe_with_next = true;
            rp.background = true;
            createNextPart = true;
        }
        else if (input == ">")
        {
            nextIsRedirOut = true;
        }
        else if (input == "<")
        {
            nextIsRedirIn = true;
        }
        else if (input == ">>")
        {
            nextIsRedirAppend = true;
        }
        else
        {
            if (nextIsRedirIn == true)
            {
                rp.redirIn = input;
                nextIsRedirIn = false;
            }
            else if (nextIsRedirOut == true)
            {
                rp.redirOut = input;
                nextIsRedirOut = false;
            }
            else if (nextIsRedirAppend == true)
            {
                rp.redirAppend = input;
                nextIsRedirAppend = false;
            }
            else
                rp.arguments.push_back(input);
        }
        if (createNextPart)
        {
            r.parts.push_back(rp);
            rp = DAMOSH_COMMAND_PARSER_RESULT_PART();
            i = 0;
            continue;
        }
    }
    if (rp.command != "")
        r.parts.push_back(rp);
    return r;
}