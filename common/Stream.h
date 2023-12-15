//
// Created by Administrator on 2021/9/25.
//

#ifndef UNTITLED1_COMMON_STREAM_H_
#define UNTITLED1_COMMON_STREAM_H_

#include <iostream>
#include <cstring>
#include <vector>

#include "DataTypes.h"

class Stream
{
 public:
  Stream() = default;
  explicit Stream(size_t size) { streamData.resize(size); }

  void to(void *data, size_t bytes);
  void from(const void *data, size_t bytes);
  void fromStream(Stream &stream);

  std::vector<uint8_t> getStreamData() const { return streamData; }
  const uint8_t* data() const { return streamData.data(); }
  const uint8_t* pointing() const { return data() + ptr; }

  size_t getPtr() const { return ptr; }
  size_t size() const { return streamData.size(); }

  std::wstring toHexString() const { return ::toHexString(streamData.data(), streamData.size()); }
  std::wstring toBitString() const { return ::toBitString(streamData.data(), streamData.size() * 8); }

  template<typename T>
  T to()
  {
    T ret;
    to(&ret, sizeof(T));
    return ret;
  }

  template<typename T>
  void from(const T& x)
  {
    from(&x, sizeof(T));
  }

 private:
  std::vector<uint8_t> streamData;
  size_t ptr = 0;
};

#endif //UNTITLED1_COMMON_STREAM_H_
