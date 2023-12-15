//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_COMMON_DATATYPES_H_
#define UNTITLED1_COMMON_DATATYPES_H_

#include <iostream>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <list>

template<int Bits>
struct Int
{
  Int()
  {
    constexpr const int mod = Bits & 0x7;
    static_assert(!mod && Bits <= 262144, "Invalid bits");
    memset(data, 0, Bits >> 3);
  }

  uint8_t& operator [] (size_t byteIdx)
  {
    return data[byteIdx];
  }

  bool operator == (const Int &rhs) const
  {
    for (int i = 0; i < Bits >> 3; i++)
    {
      if (data[i] != rhs.data[i])
        return false;
    }
    return true;
  }

  bool operator < (const Int &rhs) const
  {
    for (int i = 0; i < Bits >> 3; i++)
    {
      if (data[i] > rhs.data[i])
        return false;
      else if (data[i] < rhs.data[i])
        return true;
    }
    return false;
  }

  uint8_t data[Bits >> 3];
};

template<int Bits>
struct IntWide
{
  IntWide()
  {
    constexpr const int mod = Bits & 63;
    static_assert(!mod && Bits >= 64 && Bits < 131072, "Invalid bits");
    memset(data, 0, Bits >> 6);
  }

  bool operator == (const IntWide &rhs) const
  {
    for (int i = 0; i < Bits >> 6; i++)
    {
      if (data[i] != rhs.data[i])
        return false;
    }
    return true;
  }

  bool operator < (const IntWide &rhs) const
  {
    for (int i = 0; i < Bits >> 6; i++)
    {
      if (data[i] > rhs.data[i])
        return false;
      else if (data[i] < rhs.data[i])
        return true;
    }
    return false;
  }

  uint64_t data[Bits >> 6];
};

struct Int128
{
  bool operator == (const Int128 &rhs) const
  {
    return (h == rhs.h) && (l == rhs.l);
  }

  bool operator < (const Int128 &rhs) const
  {
    return (h > rhs.h) ?
           false :
           (h == rhs.h) ?
           l < rhs.l :
           true;
  }

  uint64_t h = 0, l = 0;
};

static std::wstring toBitString(const void *data, size_t bits)
{
  auto p = reinterpret_cast<const uint8_t*>(data);
  std::wstringstream ss;
  ss << "[";
  for (size_t i = 0; i < bits; i += 8)
  {
    for (size_t j = 0, d = p[i >> 3]; j < std::min(size_t(8), bits - i); j++, d >>= 1)
      ss << (d & 1);
    ss << " ";
  }
  ss << "]";
  return ss.str();
}

static std::wstring toHexString(const void *data, size_t bytes)
{
  auto p = reinterpret_cast<const uint8_t*>(data);
  std::wstringstream ss;
  ss << "[";
  for (size_t i = 0; i < bytes; i++)
  {
    ss << std::hex << std::setw(2) << std::setfill(L'0') << int(p[i]);
    if (i != bytes - 1)
      ss << " ";
  }
  ss << "]";
  return ss.str();
}

template<typename T>
std::wstring toBitString(const T &value)
{
  return toBitString(&value, size_t(8) * sizeof(T));
}

template<typename T>
std::wstring toHexString(const T &value)
{
  return toHexString(&value, sizeof(T));
}

#endif //UNTITLED1_COMMON_DATATYPES_H_
