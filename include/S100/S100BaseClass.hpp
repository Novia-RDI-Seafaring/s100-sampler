#pragma once

#include <string>
#include <optional>
#include <vector>
#include "DirectPosition.hpp"

class S100BaseDataClass
{
public:
    virtual ~S100BaseDataClass() = default;
    virtual DirectPosition* getFirstPosition() const = 0;
};
