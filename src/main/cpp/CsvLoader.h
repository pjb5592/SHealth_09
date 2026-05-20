#pragma once

#include <string>
#include <vector>

#include "PersonRecord.h"

namespace shealth {
namespace detail {
namespace csv {

std::vector<std::string> split(const std::string& line, char delimiter);
bool loadFromCsv(const std::string& filename, std::vector<PersonRecord>& records, int maxRecords);

}  // namespace csv
}  // namespace detail
}  // namespace shealth
