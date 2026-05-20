#pragma once

#include <vector>

#include "PersonRecord.h"

namespace shealth {
namespace detail {
namespace impute {

void fillWeightZeros(std::vector<PersonRecord>& records);
void fillHeightZeros(std::vector<PersonRecord>& records);

}  // namespace impute
}  // namespace detail
}  // namespace shealth
