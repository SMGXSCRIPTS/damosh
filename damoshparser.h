#ifndef DAMOSHPARSER_H
#define DAMOSHPARSER_H
#include <string>
#include <vector>
using namespace std;

struct DAMOSH_COMMAND_PARSER_RESULT_PART
{
    string command;
    vector<string> arguments;
    bool background;
    bool pipe_with_next;
    string redirIn;
    string redirOut;
    string redirAppend;
    
    DAMOSH_COMMAND_PARSER_RESULT_PART()
    {
        background = false;
        pipe_with_next = false;
        redirIn = "";
        redirOut = "";
        redirAppend = "";
    }
    string getAbsolutePath(string searchPath);
};

struct DAMOSH_COMMAND_PARSER_RESULT
{
    vector<DAMOSH_COMMAND_PARSER_RESULT_PART> parts;
};

class DAMOSH_COMMAND_PARSER
{
    public:
    static DAMOSH_COMMAND_PARSER_RESULT parse(string command);
};
#endif