#pragma once
#include "gpdefine.h"
#include <cstdint>
#include <string>

GP_NAMESPACE_BEGIN

template <typename _Value_TYPE>
struct Value {
  virtual bool Parser(const std::string &) = 0;
  bool valid=false;
  _Value_TYPE value = _Value_TYPE();
};

enum InputStatus { ErrorOccur = GP_ERROR, Geting, Geted };
template<typename _TYPE>
class GPBase {
public:
  GPBase(){};
 virtual ~GPBase(){};
  virtual InputStatus Input(const _TYPE&) { return InputStatus::ErrorOccur; }
 virtual void Reset(){};
  protected :
  std::string status_buffer = std::string();
};

GP_NAMESPACE_END


