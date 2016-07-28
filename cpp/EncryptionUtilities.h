#ifndef ENCRYPTION_UTILITIES_H_
#define ENCRYPTION_UTILITIES_H_

namespace rct {
class REACTOR_API EncryptionUtilities
{
public:
    static bool Encrypt3DES(const std::string& data, const std::string& key, bool uuEncode, std::string& result);
    static bool Decrypt3DES(const std::string& data, const std::string& key, bool uuDecode, std::string& result);
};

}
#endif //ENCRYPTION_UTILITIES_H_