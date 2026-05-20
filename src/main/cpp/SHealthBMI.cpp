#include "SHealth.h"

#include "SHealthBmiReport.h"

#include <cstdio>

int main() {
    SHealth shealth;
    shealth.calculateBmi("shealth.dat");

    const std::string report = shealth::report::formatCohortBmiRatioReport(shealth);
    std::fputs(report.c_str(), stdout);

    return 0;
}
