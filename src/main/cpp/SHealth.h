#pragma once

#include <array>
#include <string>
#include <vector>

class SHealth {
public:
    enum class BmiCategoryType : int {
        Underweight = 100,
        Normal = 200,
        Overweight = 300,
        Obesity = 400,
    };

    int calculateBmi(const std::string& filename);
    double getBmiRatio(int ageClass, int type);

private:
    static constexpr int kMaxRecords = 10000;
    static constexpr int kAgeClassMin = 20;
    static constexpr int kAgeClassMax = 70;
    static constexpr int kAgeClassStep = 10;
    static constexpr int kAgeCohortCount =
        (kAgeClassMax - kAgeClassMin) / kAgeClassStep + 1;
    static constexpr int kBmiCategoryCount = 4;

    static constexpr double kBmiUnderweightMax = 18.5;
    static constexpr double kBmiNormalUpperExclusive = 23.0;
    static constexpr double kBmiOverweightMin = 23.0;
    static constexpr double kBmiOverweightMaxExclusive = 25.0;
    static constexpr double kBmiObesityMinExclusive = 25.0;

    static constexpr double kCmPerMeter = 100.0;
    static constexpr double kPercentScale = 100.0;

    enum class BmiCategoryIndex : int {
        Underweight = 0,
        Normal = 1,
        Overweight = 2,
        Obesity = 3,
    };

    int count = 0;
    int ages[kMaxRecords];
    double heights[kMaxRecords];
    double weights[kMaxRecords];
    double bmis[kMaxRecords];

    std::array<std::array<double, kBmiCategoryCount>, kAgeCohortCount> cohortRatios_{};

    bool loadFromCsv(const std::string& filename);
    void imputeMissingWeights();
    void computeAllBmi();
    void computeAgeCohortRatios();

    static bool isInAgeCohort(int age, int ageClass);
    static int ageClassToCohortIndex(int ageClass);
    static int typeToCategoryIndex(int type);
    static int classifyBmiCategory(double bmi);
    static double computeBmi(double weightKg, double heightCm);

    std::vector<std::string> split(const std::string& line, char delimiter);
};
