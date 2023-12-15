//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_COMPRESS_COMPRESS_H_
#define UNTITLED1_COMPRESS_COMPRESS_H_

#include "HuffmanTree.h"
#include "../common/BitStream.h"
#include "../common/Stream.h"

struct CompressedFileHeader
{
  uint32_t rawSize;
  uint16_t keyLengthInBytes;
  uint32_t nKeyPairs;
};

template<typename T>
void getCodeRecursive(HuffmanNode<T> *k, BitStream &code, std::map<T, BitStream> &codeMap)
{
  if (k->word.has_value())
  {
//    std::wcout << toHexString(k->word.value()) << " " << k->freq << " " << code.toBitString() << "\n";
    codeMap[k->word.value()] = code;
  }
  else
  {
    code.addBit(0);
    getCodeRecursive(k->lch, code, codeMap);
    code.removeBit();

    code.addBit(1);
    getCodeRecursive(k->rch, code, codeMap);
    code.removeBit();
  }
}

template<typename T>
void keyPairsToBitStreamDfsOrder(HuffmanNode<T> *k, BitStream &keyBits, BitStream &treeBits)
{
  if (k->word.has_value())
    keyBits << k->word.value();
  else
  {
    treeBits.addBit(0);
    treeBits.addBit(k->lch->word.has_value());
    keyPairsToBitStreamDfsOrder(k->lch, keyBits, treeBits);
    treeBits.addBit(1);
    treeBits.addBit(k->rch->word.has_value());
    keyPairsToBitStreamDfsOrder(k->rch, keyBits, treeBits);
  }
}

template<typename T>
BitStream compress(const HuffmanTree<T> &tree, const void *rawData, size_t rawSize)
{
  std::map<T, BitStream> codeMap;
  auto root = tree.getRoot();
  BitStream initCode;

  if (root->height == 1)
    initCode.addBit(0);
//  std::wcout << "<<Code Table>>\n";
  getCodeRecursive(root, initCode, codeMap);

  BitStream bs;
  size_t maxCodeLength = 0;

  for (const auto &key : codeMap)
    maxCodeLength = std::max(maxCodeLength, key.second.sizeByBit());

  CompressedFileHeader header;
  header.rawSize = rawSize;
  header.keyLengthInBytes = sizeof(T);
  header.nKeyPairs = codeMap.size();
  bs << header;

//  std::wcout << toBitString(header) << "\n";

  BitStream keyBits, treeBits;
  keyPairsToBitStreamDfsOrder(root, keyBits, treeBits);

//  std::wcout << "<<KeyBits>>\n";
//  std::wcout << keyBits.toBitString() << "\n";

  if (root->height == 1)
  {
    treeBits.addBit(0);
    treeBits.addBit(1);
  }

//  std::wcout << "<<TreeBits>>\n";
//  std::wcout << treeBits.toBitString() << "\n";

  bs.addFromBitStream(keyBits);
  bs.addFromBitStream(treeBits);

  auto pData = reinterpret_cast<const uint8_t*>(rawData);
  for (size_t i = 0; i < rawSize; i += sizeof(T))
  {
    T value;
    auto pValue = reinterpret_cast<uint8_t*>(&value);
    for (size_t j = i; j < std::min(i + sizeof(T), rawSize); j++)
      pValue[j - i] = pData[j];
    bs.addFromBitStream(codeMap[value]);
  }

  auto compressedSize = (bs.sizeByBit() + 7) >> 3;
  std::cout << "<<Compressed size: " << bs.sizeByBit() << " bits>>\n";
  std::cout << rawSize << "B -> " << compressedSize << "B, " << static_cast<float>(compressedSize) / rawSize * 100.0f << "%\n";
  return bs;
}

template<typename T>
BitStream compress(const void *rawData, size_t rawSize)
{
  HuffmanTree<T> tree(rawData, rawSize);
  return compress(tree, rawData, rawSize);
}

template<typename T>
Stream compress(const Stream &stream)
{
  Stream ret;
  BitStream bs = compress<T>(stream.data(), stream.size());
  ret.from(bs.data(), bs.sizeByByte());
  return ret;
}

template<typename T>
BitStream decompress(BitStream &bs)
{
  BitStream res;
  CompressedFileHeader header;
  bs >> header;
  std::wcout << "<<RawSizeInBytes: " << header.rawSize << ", KeyLengthInBytes: " << header.keyLengthInBytes
            << ", nKeyPairs: " << header.nKeyPairs << ">>\n";

  T *keys = new T[header.nKeyPairs];

  for (uint32_t i = 0; i < header.nKeyPairs; i++)
    bs >> keys[i];

//  std::wcout << "<<Recovered Keys>>\n";
//  for (uint32_t i = 0; i < header.nKeyPairs; i++)
//    std::wcout << toHexString(keys[i]) << " ";
//  std::wcout << "\n";

  if (header.nKeyPairs > 1)
  {
    HuffmanTree<T> tree(bs, keys, header.nKeyPairs);
    while (!bs.finished())
      res << tree.match(bs);
  }
  else
  {
    for (size_t i = bs.posByBit() + 2; i < bs.sizeByBit(); i++)
      res << keys[0];
  }
  delete[] keys;
  res.setSizeByBit(header.rawSize << 3);
  return res;
}

template<typename T>
BitStream decompress(const void *data, size_t size)
{
  BitStream comp;
  comp.addBits(data, size << 3);
  return decompress<T>(comp);
}

template<typename T>
Stream decompress(const Stream &stream)
{
  Stream ret;
  BitStream bs = decompress<T>(stream.data(), stream.size());
  ret.from(bs.data(), bs.sizeByByte());
  return ret;
}

#endif //UNTITLED1_COMPRESS_COMPRESS_H_
