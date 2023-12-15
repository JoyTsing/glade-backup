//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_COMPRESS_HUFFMANTREE_H_
#define UNTITLED1_COMPRESS_HUFFMANTREE_H_

#include <iostream>
#include <optional>
#include <fstream>
#include <vector>
#include <queue>
#include <stack>
#include <list>
#include <map>

#include "../common/BitStream.h"

template<typename T>
struct HuffmanNode
{
  HuffmanNode() = default;
  HuffmanNode(HuffmanNode *lch, HuffmanNode *rch) :
      lch(lch), rch(rch), freq(lch->freq + rch->freq) {}
  HuffmanNode(const T &word, uint64_t freq) :
      word(word), freq(freq), height(1) {}

  HuffmanNode *lch = nullptr;
  HuffmanNode *rch = nullptr;
  std::optional<T> word;
  uint64_t freq;
  uint32_t height;
};

template<typename T>
struct HuffmanNodeCmp
{
  explicit HuffmanNodeCmp(HuffmanNode<T> *node) : node(node) {}

  bool operator < (const HuffmanNodeCmp &rhs) const
  {
    return node->freq > rhs.node->freq;
  }
  HuffmanNode<T> *node;
};

template<typename T>
class HuffmanTree
{
 public:
  HuffmanTree(const void *rawData, size_t rawSize)
  {
    auto pData = reinterpret_cast<const uint8_t*>(rawData);
    // TODO: handle empty file
    for (size_t i = 0; i < rawSize; i += sizeof(T))
    {
      T value;
      for (size_t j = i; j < std::min(i + sizeof(T), rawSize); j++)
        *(reinterpret_cast<uint8_t*>(&value) + j - i) = pData[j];

      if (freqCount.find(value) == freqCount.end())
        freqCount[value] = 1;
      else
        freqCount[value]++;
    }
    std::priority_queue<HuffmanNodeCmp<T>> heap;

    for (const auto &key : freqCount)
    {
      auto *node = new HuffmanNode<T>(key.first, key.second);
      heap.push(HuffmanNodeCmp<T>(node));
    }

    if (heap.size() == 1)
    {
      root = heap.top().node;
      return;
    }

    while (!heap.empty())
    {
      auto l = heap.top().node;
      heap.pop();
      auto r = heap.top().node;
      heap.pop();

      auto *p = new HuffmanNode<T>(l, r);
      p->height = std::max(l->height, r->height) + 1;
      if (heap.empty())
      {
        root = p;
        break;
      }
      heap.push(HuffmanNodeCmp<T>(p));
    }
  }

  HuffmanTree(BitStream &codeBits, T *keys, uint32_t nKeyPairs)
  {
    root = new HuffmanNode<T>();
    uint32_t index = 0;

    if (nKeyPairs == 1)
    {
      root->word = *keys;
      return;
    }
    buildFromCodeBitsRecursive(root, codeBits, keys, index, nKeyPairs);
  }

  ~HuffmanTree()
  {
    if (root)
      destroyRecursive(root);
  }

  T match(BitStream &codeBits)
  {
    HuffmanNode<T> *k = root;
    while (!k->word)
      k = codeBits.readBit() ? k->rch : k->lch;
    return k->word.value();
  }

  HuffmanNode<T>* getRoot() const { return root; }

  std::map<T, uint64_t> getFreqCount() const { return freqCount; }

 private:
  void buildFromCodeBitsRecursive(HuffmanNode<T> *k, BitStream &codeBits, T *keys, uint32_t &index, uint32_t nKeyPairs)
  {
    k->lch = new HuffmanNode<T>();
    k->rch = new HuffmanNode<T>();

    uint8_t lCode = codeBits.readBits<2>();
    if (!(lCode & 1))
      buildFromCodeBitsRecursive(k->lch, codeBits, keys, index, nKeyPairs);
    else
    {
      k->lch->word = keys[index++];
      //std::cout << toBitString(k->lch->word.value()) << "\n";
    }
    uint8_t rCode = codeBits.readBits<2>();
    if (!(rCode & 1))
      buildFromCodeBitsRecursive(k->rch, codeBits, keys, index, nKeyPairs);
    else
    {
      k->rch->word = keys[index++];
      //std::cout << toBitString(k->rch->word.value()) << "\n";
    }
  }

  void destroyRecursive(HuffmanNode<T> *&k)
  {
    if (!k->word)
    {
      destroyRecursive(k->lch);
      if (k->rch)
        destroyRecursive(k->rch);
    }
    delete k;
  }

 private:
  HuffmanNode<T> *root = nullptr;
  std::map<T, uint64_t> freqCount;
};

#endif //UNTITLED1_COMPRESS_HUFFMANTREE_H_
