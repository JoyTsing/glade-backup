//
// Created by Administrator on 2021/9/25.
//

#include "Stream.h"

void Stream::to(void *data, size_t bytes)
{
  auto p = reinterpret_cast<uint8_t*>(data);
  memcpy(p, streamData.data() + ptr, bytes);
  ptr += bytes;
}

void Stream::from(const void *data, size_t bytes)
{
  auto p = reinterpret_cast<const uint8_t*>(data);
  streamData.insert(streamData.end(), p, p + bytes);
}

void Stream::fromStream(Stream &stream)
{
  from(stream.data(), stream.size() - stream.getPtr());
}
