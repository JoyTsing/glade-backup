//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_COMMON_BITSTREAM_H_
#define UNTITLED1_COMMON_BITSTREAM_H_

#include <iostream>
#include <fstream>
#include <vector>

#include "DataTypes.h"

class BitStream
{
 public:
  BitStream() = default;
  BitStream(const void *data, size_t bits);
  //BitStream(const char *filePath);

  void addBit(uint8_t bit);
  void addByte(uint8_t byte);
  void addBits(const void *data, size_t length);
  void addFromBitStream(const BitStream &bs) { addBits(bs.bitData.data(), bs.bits); }

  void removeBit();
  uint8_t readBit();
  uint8_t readByte();

  BitStream readToBitStream(size_t sizeInBits);

  void resize() { bitData.resize((bits + 7) >> 3); }
  void clear();

  std::wstring toBitString() const { return ::toBitString(bitData.data(), bits); }
  std::wstring toHexString() const { return ::toHexString(bitData.data(), (bits + 7) >> 3); }

  std::vector<uint8_t> getBitData() const { return bitData; }

  const uint8_t* data() const { return bitData.data(); }

  void setSizeByBit(size_t size) { bits = size; }
  size_t sizeByBit() const { return bits; }
  size_t sizeByByte() const { return (bits + 7) >> 3; }
  void setPosByBit(size_t pos) { ptr = pos; }
  size_t posByBit() const { return ptr; }
  bool finished() const { return ptr >= bits; }

  bool operator == (const BitStream &rhs) const;

  template<typename T>
  void add(const T &v)
  {
    addBits(&v, sizeof(T) * 8);
  }

  template<typename T>
  BitStream& operator << (const T &v)
  {
    add(v);
    return *this;
  }

  template<int Bits>
  uint8_t readBits()
  {
    static_assert(Bits < 9 && Bits > 0, "Bits out of range");
    uint8_t ret = 0;
    for (int i = 0; i < Bits; i++)
      ret = ret << 1 | readBit();
    return ret;
  }

  template<typename T>
  BitStream& operator >> (T &v)
  {
    uint8_t mod = ptr & 0b111;
    auto *p = (uint8_t*)&v;
    for (int i = 0; i < sizeof(T); i++)
    {
      uint8_t byte = bitData[ptr >> 3] >> mod;
      if (!mod)
        byte |= bitData[(ptr >> 3) + 1] << (8 - mod);
      p[i] = byte;
      ptr += 8;
    }
    return *this;
  }

 private:
  std::vector<uint8_t> bitData;
  size_t bits = 0;
  size_t ptr = 0;
};

#endif //UNTITLED1_COMMON_BITSTREAM_H_
