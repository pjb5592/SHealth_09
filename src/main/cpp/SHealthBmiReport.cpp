#include "SHealthBmiReport.h"

#include "BmiDomain.h"
#include "SHealth.h"

#include <cstdio>
#include <string>

namespace shealth::report {

std::string formatCohortBmiRatioReport(const SHealth& health) {
    std::string report;
    char buffer[512];

    for (const domain::AgeCohortDescriptor& cohort : domain::kAgeCohorts) {
        const auto& firstCategory = domain::kBmiCategories[0];
        int written = std::snprintf(
            buffer, sizeof(buffer), "%d - %s = %f", cohort.ageClass, firstCategory.label,
            health.getBmiRatio(cohort.ageClass, static_cast<int>(firstCategory.apiType)));
        report.append(buffer, static_cast<std::size_t>(written));

        for (std::size_t i = 1; i < domain::kBmiCategoryTableSize; ++i) {
            const auto& category = domain::kBmiCategories[i];
            written = std::snprintf(buffer, sizeof(buffer), ", %s = %f", category.label,
                                    health.getBmiRatio(cohort.ageClass,
                                                       static_cast<int>(category.apiType)));
            report.append(buffer, static_cast<std::size_t>(written));
        }
        report.push_back('\n');
    }

    return report;
}

}  // namespace shealth::report
