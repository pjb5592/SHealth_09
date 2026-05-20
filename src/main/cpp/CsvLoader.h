#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "PersonRecord.h"

namespace shealth {
namespace detail {
namespace csv {

// DEF-007: 한 번의 loadFromCsv 호출당 최대 데이터 행 수. 초과 행은 무시(truncate)하며 OOB 없음.
// shealth.dat(≈4822건)는 상한 미만 — 기존 동작·baseline 유지.
constexpr int kMaxCsvRecords = 10000;

std::vector<std::string> split(const std::string& line, char delimiter);
bool loadFromCsv(std::istream& input, std::vector<PersonRecord>& records, int maxRecords);
bool loadFromCsv(const std::string& filename, std::vector<PersonRecord>& records, int maxRecords);

}  // namespace csv
}  // namespace detail
}  // namespace shealth
