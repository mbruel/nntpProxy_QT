#ifndef MYCRYPT_H
#define MYCRYPT_H

typedef unsigned short ushort;
#include <string>

#define LENGTH_MULTIPLIER  1

class MyCrypt{
public:
    explicit MyCrypt(const std::string & aKey);
    MyCrypt(const MyCrypt &)              = delete;
    MyCrypt(const MyCrypt &&)             = delete;
    MyCrypt & operator=(const MyCrypt &)  = delete;
    MyCrypt & operator=(const MyCrypt &&) = delete;

    bool enc(const std::string & aMsgToEnc, std::string & enc, ushort aLengthMultiplier = LENGTH_MULTIPLIER) const;
    bool dec(const std::string & aMsgToDec, std::string & dec, ushort aLengthMultiplier = LENGTH_MULTIPLIER) const;

private:
    const std::string iKey;
};

#endif // MYCRYPT_H
