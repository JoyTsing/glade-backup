//
// Created by Administrator on 2021/9/24.
//

#include "Win32Util.h"

NAMESPACE_BEGIN(util)

uint64_t toQword(DWORD high, DWORD low)
{
  return uint64_t(high) << 32 | low;
}

uint64_t toQword(FILETIME time)
{
  return toQword(time.dwHighDateTime, time.dwLowDateTime);
}

uint64_t hash(uint64_t seed)
{
  uint64_t x = seed;
  x ^= x >> 12;
  x ^= x << 25;
  x ^= x >> 27;
  return x * 0x2545f4914f6cdd1dull;
}

uint64_t hash(const void *data, size_t bytes)
{
  auto p8Data = reinterpret_cast<const uint8_t*>(data);
  auto p64Data = reinterpret_cast<const uint64_t*>(data);
  uint64_t res = 0;
  uint64_t seed = 1;
  size_t n = (bytes + sizeof(uint64_t) - 1) / sizeof(uint64_t);
  for (size_t i = 0; i < n; i++)
  {
    res = res * seed + p64Data[i];
    seed = hash(seed);
  }
  int mod = n % sizeof(uint64_t);
  if (mod)
  {
    uint64_t last = 0;
    for (int i = 0; i < mod; i++)
    {
      res = res * seed + p8Data[n * sizeof(uint64_t) + i];
      seed = hash(seed);
    }
  }
  return res;
}

uint64_t getTime()
{
  SYSTEMTIME sysTime;
  GetLocalTime(&sysTime);
  FILETIME fTime;
  SystemTimeToFileTime(&sysTime, &fTime);
  return toQword(fTime.dwHighDateTime, fTime.dwLowDateTime);
}

void forceCloseAllHandles()
{
  for (auto handle : handleHistory)
    CloseHandle(handle);
}

NAMESPACE_END