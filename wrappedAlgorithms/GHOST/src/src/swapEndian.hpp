#pragma once
#include <algorithm>
  
template <typename T>
void swap_endian(T& pX)
{
  char& raw = reinterpret_cast<char&>(pX);
  std::reverse(&raw, &raw + sizeof(T));
}


