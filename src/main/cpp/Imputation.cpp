#include "Imputation.h"

#include "BmiDomain.h"

namespace shealth {
namespace detail {
namespace impute {

namespace domain = shealth::domain;

namespace {

bool isZero(double value) { return value == 0.0; }

void fillCohortAverage(std::vector<PersonRecord>& records,
                       double PersonRecord::*field,
                       bool (*isMissing)(double)) {
    const std::size_t count = records.size();
    for (const domain::AgeCohortDescriptor& cohort : domain::kAgeCohorts) {
        double sum = 0;
        int validCount = 0;
        for (std::size_t i = 0; i < count; i++) {
            if (!domain::isAgeInCohort(records[i].age, cohort)) {
                continue;
            }
            const double value = records[i].*field;
            if (isMissing(value)) {
                continue;
            }
            sum += value;
            validCount++;
        }
        if (validCount == 0) {
            continue;
        }
        const double average = sum / validCount;
        for (std::size_t i = 0; i < count; i++) {
            if (!domain::isAgeInCohort(records[i].age, cohort)) {
                continue;
            }
            if (isMissing(records[i].*field)) {
                records[i].*field = average;
            }
        }
    }
}

}  // namespace

void fillWeightZeros(std::vector<PersonRecord>& records) {
    fillCohortAverage(records, &PersonRecord::weight, isZero);
}

void fillHeightZeros(std::vector<PersonRecord>& records) {
    fillCohortAverage(records, &PersonRecord::height, isZero);
}

}  // namespace impute
}  // namespace detail
}  // namespace shealth
