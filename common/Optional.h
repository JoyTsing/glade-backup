//
// Created by Administrator on 2021/9/24.
//

#ifndef UNTITLED1_COMMON_OPTIONAL_H_
#define UNTITLED1_COMMON_OPTIONAL_H_

#include <iostream>

template<typename T>
class Optional
{
 public:
  Optional() = default;

  explicit Optional(const T &val)
  {
    data = new T(val);
  }

  ~Optional()
  {
    if (data)
      delete data;
  }

  T value()
  {
    if (!data)
      throw "Optional::no value";
    return *data;
  }

  Optional& operator = (const T &val)
  {
    data = new T(val);
    return *this;
  }

  bool operator ! () const
  {
    return data == nullptr;
  }

  bool has_value() const
  {
    return data != nullptr;
  }

 private:
  T *data = nullptr;
};

#endif //UNTITLED1_COMMON_OPTIONAL_H_
