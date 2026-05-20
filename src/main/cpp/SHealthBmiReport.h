#pragma once

#include <string>

class SHealth;

namespace shealth::report {

// SHealthBMI main과 동일한 6연령×4분류 printf 포맷을 문자열로 생성한다.
std::string formatCohortBmiRatioReport(const SHealth& health);

}  // namespace shealth::report
