#include "nntp.h"

std::map<unsigned short, const char *> Nntp::sResponses{};
std::map<Nntp::CMDS, std::regex>      Nntp::sCmdRegex{};

Nntp::Nntp() {}

const char * Nntp::getResponse(unsigned short aCode){
    try {
        return sResponses.at(aCode);
    } catch (std::out_of_range ex) {
        return sResponses.at(0);
    }
}

const std::regex & Nntp::getCmdRegex(Nntp::CMDS aCmd){
    return sCmdRegex.at(aCmd);
}

void Nntp::initMaps(){
    setResponsesMap();
    setCmdRegexMap();
}

void Nntp::setCmdRegexMap(){
    sCmdRegex[Nntp::CMDS::authinfo] = std::regex("^\\s*authinfo\\s+(user|pass)\\s+(.*)(\r)?\n$",
                                                 std::regex_constants::ECMAScript  | std::regex_constants::icase);
    sCmdRegex[Nntp::CMDS::group]    = std::regex("^\\s*group\\s+(\\w*)(\r)?\n$",
                                                 std::regex_constants::ECMAScript  | std::regex_constants::icase);

}

void Nntp::setResponsesMap(){
    sResponses[0]   = "";

    //rfc977: 2.4.3.  General Responses
    sResponses[100] = "100 help text follows\r\n";
    sResponses[200] = "200 NG_LinK servers ready - posting allowed\r\n";
    sResponses[201] = "201 NG_LinK servers ready - no posting allowed\r\n";
    sResponses[400] = "400 service discontinued\r\n";
    sResponses[500] = "500 command not recognized\r\n";
    sResponses[501] = "501 command syntax error\r\n";
    sResponses[502] = "502 access restriction or permission denied\r\n";
    sResponses[503] = "503 program fault - command couldn't perform\r\n";


    //rfc977: 3.1.  The ARTICLE, BODY, HEAD, and STAT commands
    sResponses[220] = "220 n <a> article retrieved - head and body follow (n = article number, <a> = message-id)\r\n";
    sResponses[221] = "221 n <a> article retrieved - head follows\r\n";
    sResponses[222] = "222 n <a> article retrieved - body follows\r\n";
    sResponses[223] = "223 n <a> article retrieved - request text separately\r\n";
    sResponses[412] = "412 no newsgroup has been selected\r\n";
    sResponses[420] = "420 no current article has been selected\r\n";
    sResponses[423] = "423 no such article number in this group\r\n";
    sResponses[430] = "430 no such article found\r\n";


    //rfc977: 3.2.  The GROUP command
    sResponses[211] = "211 <number of articles> <first one> <last one> <name of the group>\r\n";
    sResponses[411] = "411 no such news group\r\n";

    //rfc977: 3.5.  The LAST command (223, 412,420,422)
    sResponses[422] = "422 no previous article in this group\r\n";

    //rfc977: 3.0.  The NEXT command (223, 412,420,421)
    sResponses[421] = "421 no next article in this group\r\n";

    //rfc977: 3.6.  The LIST command
    sResponses[215] = "215 list of newsgroups follows\r\n";

    //rfc977: 3.11.  The QUIT command
    sResponses[205] = "205 Good Bye, thanks for using NG_LinK\r\n";


    sResponses[480] = "480 Authentication Required\r\n";
    sResponses[380] = "380 More Authentication Required\r\n";
    sResponses[381] = "381 More Authentication Required\r\n";
    sResponses[281] = "281 Welcome to NG_LinK - Enjoy unlimited Usenet downloads!\r\n";
}


