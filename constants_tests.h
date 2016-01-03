#ifndef CONSTANTS_TESTS
#include "constants.h"

#define CONSTANTS_TESTS

NntpServerParameters cTestNntpServParam(){
    return NntpServerParameters(
                "news.myprovider.com", 119, true,
                "my encrypted login", "my encrypted pass", 2, false);
}

NntpServerParameters cTestNntpServParamSSL(){
    return NntpServerParameters(
                "news.myprovider.com", 443, true,
                "my encrypted login", "my encrypted pass", 3, true);
}

DatabaseParameters cTestDbParams(){
    return DatabaseParameters(
                "QMYSQL", "localhost", 6606,
                "my encrypted login", "my encrypted pass", "nntpProxy");
}

static const constexpr char* cGoodUserNntp = "my DB user";
static const constexpr char* cGoodPassNntp = "my DB user password";

#endif // CONSTANTS_TESTS

