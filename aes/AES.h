//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_AES_AES_H_
#define UNTITLED1_AES_AES_H_

#include <iostream>
#include <optional>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>

#include "AESMatrices.h"
#include "../common/DataTypes.h"
#include "../common/Stream.h"

NAMESPACE_BEGIN(aes)

const int ROUND_KEY_EXPANSION = 11;
const int BLOCK_SIZE = 16;

enum class Mode
{
  Encrypt, Decrypt
};

struct Block
{
  Block() = default;
  Block(const void *p, size_t bytes);

  void clear() { std::memset(data, 0, BLOCK_SIZE); }

  void operator ^= (const Block &rhs);
  Block operator ^ (Block rhs) const;
  uint8_t& operator [] (uint8_t idx);
  bool operator != (const Block &rhs) const;

  uint32_t* p32Data();
  uint64_t* p64Data();

  static uint8_t GFMul(uint8_t a, uint8_t b);

  template<Mode mode>
  void shift()
  {
    constexpr int StartRow = (mode == Mode::Encrypt) ? 0 : 3;
    constexpr int EndRow = (mode == Mode::Encrypt) ? 3 : 0;
    constexpr int Dr = (mode == Mode::Encrypt) ? 1 : -1;

    auto pData = p32Data();
    for (int i = 1; i < 4; i++)
    {
      uint32_t maskH = 0xffffffff - ((1 << (i * 8)) - 1);
      uint32_t maskL = ~maskH;
      uint32_t tmp = pData[StartRow];
      for (int j = StartRow; j != EndRow; j += Dr)
        pData[j] = (pData[j + Dr] & maskH) | (pData[j] & maskL);
      pData[EndRow] = (tmp & maskH) | (pData[EndRow] & maskL);
    }
  }

  template<Mode mode>
  void replaceBytes()
  {
    constexpr const uint8_t *pBoxData = (mode == Mode::Encrypt) ? S_BOX : S_BOX_INV;
    for (auto &i : data)
      i = pBoxData[i];
  }

  template<Mode mode>
  void mix()
  {
    static const uint8_t Mat[BLOCK_SIZE] =
        {
            0x02, 0x03, 0x01, 0x01, 0x01, 0x02, 0x03, 0x01,
            0x01, 0x01, 0x02, 0x03, 0x03, 0x01, 0x01, 0x02
        };
    static const uint8_t MatInv[BLOCK_SIZE] =
        {
            0x0e, 0x0b, 0x0d, 0x09, 0x09, 0x0e, 0x0b, 0x0d,
            0x0d, 0x09, 0x0e, 0x0b, 0x0b, 0x0d, 0x09, 0x0e
        };
    constexpr const uint8_t *p8Mat = (mode == Mode::Encrypt) ? Mat : MatInv;

    uint64_t tmp[2] = { 0, 0 };
    auto p8Tmp = reinterpret_cast<uint8_t*>(tmp);

    for (int i = 0; i < 4; i++)
    {
      for (int k = 0; k < 4; k++)
      {
        uint8_t res = 0;
        for (int j = 0; j < 4; j++)
          res ^= GFMul(p8Mat[k * 4 + j], data[i * 4 + j]);
        p8Tmp[i * 4 + k] = res;
      }
    }
    p64Data()[0] = tmp[0];
    p64Data()[1] = tmp[1];
  }

  uint8_t data[BLOCK_SIZE];
};

struct FileHeader
{
  size_t rawSize;
  Block cryptedKey;
};

using ExpandedKey = std::array<Block, ROUND_KEY_EXPANSION>;
ExpandedKey expandKey(const Block &key);

template<Mode mode>
Block cryptBlock(Block block, const Block &key)
{
  const int NumRounds = ROUND_KEY_EXPANSION;
  auto expandedKeys = expandKey(key);

  constexpr int Start = (mode == Mode::Encrypt) ? 0 : NumRounds - 1;
  constexpr int End = (mode == Mode::Encrypt) ? NumRounds : -1;
  constexpr int Dr = (mode == Mode::Encrypt) ? 1 : -1;

  for (int i = Start; i != End; i += Dr)
  {
    if (mode == Mode::Encrypt)
    {
      if (i > 0)
      {
        block.replaceBytes<mode>();
        block.shift<mode>();
        if (i < NumRounds - 1)
          block.mix<mode>();
      }
      block ^= expandedKeys[i];
    }
    else
    {
      block ^= expandedKeys[i];
      if (i > 0)
      {
        if (i < NumRounds - 1)
          block.mix<mode>();
        block.shift<mode>();
        block.replaceBytes<mode>();
      }
    }
  }
  return block;
}

template<Mode mode>
void crypt(std::vector<Block> &textBlocks, const Block &key)
{
  for (auto &block : textBlocks)
    block = cryptBlock<mode>(block, key);
}

Stream encrypt(const Stream &stream, const std::string &key);
std::optional<Stream> decrypt(Stream &stream, const std::string &key);

NAMESPACE_END

#endif //UNTITLED1_AES_AES_H_
