#pragma once
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1

#include <cryptopp/md5.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>

/**
 * @brief 对输入串进行md5加密得到加密串
 * @param[in] str 输入串
 * @return 加密串
 */
std::string md5(const std::string& str);

