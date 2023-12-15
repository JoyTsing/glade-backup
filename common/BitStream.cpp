//
// Created by Administrator on 2021/9/24.
//

#include "BitStream.h"

BitStream::BitStream(const void *data, size_t bits) :
    bits(bits)
{
  size_t bytes = (bits + 7) >> 3;
  bitData.resize(bytes);
  std::memcpy(bitData.data(), data, bytes);
}

void BitStream::addBit(uint8_t bit)
{
  size_t pos = bits >> 3;
  int mod = bits & 0b111;
  if (!mod && pos >= bitData.size())
    bitData.push_back(0);
  bitData[pos] |= bit << mod;
  bits++;
}

void BitStream::addByte(uint8_t byte)
{
  size_t pos = bits >> 3;
  int mod = bits & 0b111;
  bitData.push_back(byte >> (8 - mod));
  bitData[pos] |= byte << mod;
  bits += 8;
}

void BitStream::addBits(const void *data, size_t length)
{
  auto pData = reinterpret_cast<const uint8_t*>(data);
  size_t bytes = length >> 3;
  for (size_t i = 0; i < bytes; i++)
    addByte(pData[i]);
  int mod = length & 0b111;
  for (int i = 0; i < mod; i++)
    addBit(pData[bytes] >> i);
}

void BitStream::removeBit()
{
  bits--;
  size_t pos = bits >> 3;
  uint8_t b = bitData[pos] & (1 << (bits & 0b111));
  bitData[pos] ^= b;
}

uint8_t BitStream::readBit()
{
  size_t pos = ptr >> 3;
  int mod = ptr & 0b111;
  ptr++;
  return (bitData[pos] >> mod) & 1;
}

uint8_t BitStream::readByte()
{
  size_t pos = ptr >> 3;
  int mod = ptr & 0b111;
  ptr += 8;
  if (!mod)
    return bitData[pos];
  return (bitData[pos] >> mod) | (bitData[pos + 1] << (8 - mod));
}

BitStream BitStream::readToBitStream(size_t sizeInBits)
{
  BitStream bs;
  size_t bytes = sizeInBits >> 3;
  for (size_t i = 0; i < bytes; i++)
    bs.addByte(readByte());
  int mod = sizeInBits & 0b111;
  for (size_t i = 0; i < mod; i++)
    bs.addBit(readBit());
  return bs;
}

void BitStream::clear()
{
  bits = ptr = 0;
  bitData.clear();
}

bool BitStream::operator == (const BitStream &rhs) const
{
  if (bits != rhs.bits)
    return false;
  size_t bytes = (bits + 7) >> 3;
  for (size_t i = 0; i < bytes; i++)
  {
    if (bitData[i] != rhs.bitData[i])
      return false;
  }
  return true;
}