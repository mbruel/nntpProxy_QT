#include "mycrypt.h"
MyCrypt::MyCrypt(const std::string &aKey):iKey(aKey){}

bool MyCrypt::enc(const std::string & aMsgToEnc, std::string & enc, ushort aLengthMultiplier) const{
    return true;
}

bool MyCrypt::dec(const std::string & aMsgToDec, std::string &dec, ushort aLengthMultiplier) const {
    return true;
}
