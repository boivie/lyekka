#pragma once

#include <stdint.h>
#include <cstring>
#include <iostream>

class ChunkId {
public:
  static ChunkId from_hex(const char* data);
  static ChunkId from_bin(const char* data);
  static ChunkId calculate(const void* data, size_t len);

  friend bool operator < (const ChunkId& n2, const ChunkId& n1) {
    return memcmp(n1.cid, n2.cid, 20) > 0;
  }
  friend std::ostream& operator << (std::ostream& os, const ChunkId& C);
  inline size_t hash() const { return *(size_t*)cid; };

  friend bool operator==(const ChunkId& left, const ChunkId& right) {
    return memcmp(left.cid, right.cid, 20) == 0;
  }
  const uint8_t* binary() const { return cid; }
  const std::string hex() const;
private:
  uint8_t cid[20];
};

class ChunkLocation {
public:
  ChunkLocation(uint64_t pack, uint32_t offset, uint32_t size)
    : m_location(pack << (32 - 5) | offset >> 5), m_size(size) {};
  uint64_t pack() const { return m_location >> (32 - 5); };
  uint32_t offset() const { return (uint32_t)m_location << 5; };
  uint32_t size() const { return m_size; }
private:
  const uint64_t m_location;
  const uint32_t m_size;
};
