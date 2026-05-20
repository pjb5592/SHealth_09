#pragma once

#include <array>
#include <vector>

#include "BmiDomain.h"
#include "PersonRecord.h"

namespace shealth {
namespace detail {
namespace stats {

using CohortRatios =
    std::array<std::array<double, domain::kBmiCategoryCount>, domain::kAgeCohortCount>;
using OverallRatios = std::array<double, domain::kBmiCategoryCount>;

void computeAgeCohortRatios(const std::vector<PersonRecord>& records, CohortRatios& cohortRatios);
void computeOverallRatios(const std::vector<PersonRecord>& records, OverallRatios& overallRatios);

}  // namespace stats
}  // namespace detail
}  // namespace shealth
