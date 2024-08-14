#include "md5.h"

std::string md5(const std::string& str) {
    CryptoPP::byte digest[CryptoPP::Weak::MD5::DIGESTSIZE];
    
    // 使用 Crypto++ 的 MD5 类进行计算
    CryptoPP::Weak::MD5 hash;
    hash.CalculateDigest(digest, (const CryptoPP::byte*)str.c_str(), str.length());
    
    // 将计算出的二进制哈希值转换成十六进制字符串
    // 使用 StringSource 和 HexEncoder 过滤器
    std::string md5Hex;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(md5Hex));
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();
    return md5Hex;
}
