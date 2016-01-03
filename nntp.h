#ifndef NNTP_H
#define NNTP_H

#include <map>
#include <regex>

/*!
 * \brief Pure Static class (no instance) to hold Nntp Protocol actions/responses...
 */
class Nntp
{
public:
    //! Main commands of the Nntp Protocol
    enum CMDS {quit, authinfo, group, head, body, list};

    static constexpr char* QUIT          = "quit\r\n";
    static constexpr char* AUTHINFO_USER = "authinfo user ";
    static constexpr char* AUTHINFO_PASS = "authinfo pass ";
    static constexpr char* ENDLINE       = "\r\n";

    static void initMaps(); //!< initialise the 2 static maps

    //! return the response associated to a certain code
    static const char*        getResponse(unsigned short aCode);

    //! return the regular expression to match a command
    static const std::regex & getCmdRegex(CMDS aCmd);

private:
    explicit Nntp(); // no instances
    Nntp(const Nntp &)              = delete;
    Nntp(const Nntp &&)             = delete;
    Nntp & operator=(const Nntp &)  = delete;
    Nntp & operator=(const Nntp &&) = delete;

    static void setResponsesMap(); //!< Fill the reponses map
    static void setCmdRegexMap();  //!< Fill the regex map

private:
    static std::map<unsigned short, const char *> sResponses; //!< Responses map
    static std::map<CMDS, std::regex>             sCmdRegex;  //!< Regex map
};

#endif // NNTP_H
