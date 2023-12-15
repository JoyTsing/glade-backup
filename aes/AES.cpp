//
// Created by Administrator on 2021/9/24.
//

#include "AES.h"

NAMESPACE_BEGIN(aes)

Block::Block(const void *p, size_t bytes)
{
  std::memset(data, 0, BLOCK_SIZE);
  std::memcpy(data, p, bytes);
}

void Block::operator ^= (const Block &rhs)
{
  auto rp64 = reinterpret_cast<const uint64_t*>(rhs.data);
  p64Data()[0] ^= rp64[0];
  p64Data()[1] ^= rp64[1];
}

Block Block::operator ^ (Block rhs) const
{
  rhs ^= *this;
  return rhs;
}

uint8_t& Block::operator [] (uint8_t idx)
{
  return data[idx];
}

bool Block::operator != (const Block &rhs) const
{
  for (int i = 0; i < BLOCK_SIZE; i++)
  {
    if (data[i] != rhs.data[i])
      return true;
  }
  return false;
}

uint32_t* Block::p32Data()
{
  return reinterpret_cast<uint32_t*>(data);
}

uint64_t* Block::p64Data()
{
  return reinterpret_cast<uint64_t*>(data);
}

uint8_t Block::GFMul(uint8_t a, uint8_t b)
{
  switch (a)
  {
    case 0x1:
      return MUL_1[b];
    case 0x2:
      return MUL_2[b];
    case 0x3:
      return MUL_3[b];
    case 0x9:
      return MUL_9[b];
    case 0xb:
      return MUL_B[b];
    case 0xd:
      return MUL_D[b];
    case 0xe:
      return MUL_E[b];
    default:
      break;
  }
  return 0;
}

ExpandedKey expandKey(const Block &key)
{
  const uint32_t Rcon[16] =
      {
          0x00000000, 0x01000000, 0x02000000, 0x04000000,
          0x08000000, 0x10000000, 0x20000000, 0x40000000,
          0x80000000, 0x1b000000, 0x36000000, 0x6c000000,
          0xd8000000, 0xab000000, 0xed000000, 0x9a000000
      };

  ExpandedKey res;
  res[0] = key;

  auto replace = [](uint32_t word) -> uint32_t
  {
    auto p8Word = reinterpret_cast<uint8_t*>(&word);
    for (int i = 0; i < 4; i++)
      p8Word[i] = S_BOX[p8Word[i]];
    return word;
  };

  auto rotate = [](uint32_t word) -> uint32_t
  {
    return (word >> 24) | (word << 8);
  };

  for (int i = 1; i < ROUND_KEY_EXPANSION; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      uint32_t tmp = (j == 0) ?
                     replace(rotate(res[i - 1].p32Data()[3])) ^ Rcon[i] :
                     res[i].p32Data()[j - 1];
      res[i].p32Data()[j] = tmp ^ res[i - 1].p32Data()[j];
    }
  }
  return res;
}

Stream encrypt(const Stream &stream, const std::string &key)
{
  Stream ret;
  size_t nBlocks = (stream.size() + sizeof(Block)) / sizeof(Block);

  std::vector<Block> blocks(nBlocks);
  blocks[nBlocks - 1].clear();
  memcpy(blocks.data(), stream.data(), stream.size());

  Block keyBlock(key.data(), key.length());
  auto encryptedKey = cryptBlock<Mode::Encrypt>(keyBlock, keyBlock);
  crypt<Mode::Encrypt>(blocks, keyBlock);

  ret.from(stream.size());
  ret.from(encryptedKey);
  ret.from(blocks.data(), blocks.size() * sizeof(Block));
  return ret;
}

std::optional<Stream> decrypt(Stream &stream, const std::string &key)
{
  auto rawSize = stream.to<size_t>();
  auto encryptedKey = stream.to<Block>();

  Block keyBlock(key.data(), key.size());
  auto decryptedKey = cryptBlock<Mode::Decrypt>(encryptedKey, keyBlock);
  if (decryptedKey != keyBlock)
    return std::nullopt;

  size_t nBlocks = (stream.size() - sizeof(size_t)) / sizeof(Block) - 1;
  std::vector<Block> blocks(nBlocks);
  stream.to(blocks.data(), nBlocks * sizeof(Block));
  crypt<Mode::Decrypt>(blocks, keyBlock);
  Stream out;
  out.from(blocks.data(), rawSize);
  return out;
}

NAMESPACE_END