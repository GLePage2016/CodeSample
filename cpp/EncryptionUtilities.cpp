#include "stdafx.h"
#include "Reactor.h"
#include <cryptlib.h>
#include <des.h>
#include <pssr.h>
#include <rsa.h>
#include <hex.h>
#include <base64.h>
#include <default.h>
#include "EncryptionUtilities.h"

namespace rct {

bool EncryptionUtilities::Encrypt3DES(const std::string& data, const std::string& key, bool uuEncode, std::string& result)
{
    if (data.empty() || key.empty())return(false);
    std::string encryptedData;
    CryptoPP::DefaultEncryptorWithMAC encryptor(key.data());
    CryptoPP::StringSink strSnk(encryptedData);
    encryptor.Detach(&strSnk);    
    encryptor.Put(reinterpret_cast<const unsigned char*>(data.data()), data.size());
    encryptor.MessageEnd();

    if (uuEncode)
    {
        std::string uuEncodedEncrypted;
        CryptoPP::StringSource strSrc(encryptedData, true);
        CryptoPP::Base64Encoder base64Encoder(0, false);
        CryptoPP::StringSink encodeSink(result);
        strSrc.Detach(&base64Encoder);
        strSrc.Attach(&encodeSink);
        strSrc.PumpAll();
        return(true);
    }
    else
    {
        std::string hexEncodedEncrypted;
        CryptoPP::StringSource strSrc(encryptedData, true);
        CryptoPP::HexEncoder hexEncoder(0, false);
        CryptoPP::StringSink encodeSink(result);
        strSrc.Detach(&hexEncoder);
        strSrc.Attach(&encodeSink);
        strSrc.PumpAll();
        return(true);
    }
    return(false);
}

bool EncryptionUtilities::Decrypt3DES(const std::string& data, const std::string& key, bool uuDecode, std::string& result)
{
    if (data.empty() || key.empty())return(false);
    std::string decryptedData;
    CryptoPP::DefaultDecryptorWithMAC decryptor(key.data());
    CryptoPP::StringSink strSnk(decryptedData);
    decryptor.Detach(&strSnk);    
    decryptor.Put(reinterpret_cast<const unsigned char*>(data.data()), data.size());
    decryptor.MessageEnd();

    if (uuDecode)
    {
        CryptoPP::StringSource strSrc(decryptedData, true);
        CryptoPP::Base64Decoder base64Decoder;
        CryptoPP::StringSink decodeSink(result);
        strSrc.Detach(&base64Decoder);
        strSrc.Attach(&decodeSink);
        strSrc.PumpAll();
        return(true);
    }
    else
    {
        CryptoPP::StringSource strSrc(decryptedData, true);
        CryptoPP::HexDecoder hexDecoder;
        CryptoPP::StringSink decodeSink(result);
        strSrc.Detach(&hexDecoder);
        strSrc.Attach(&decodeSink);
        strSrc.PumpAll();
        return(true);
    }
    return(false);
}

} // rct namespace