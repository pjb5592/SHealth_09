#include "CsvLoader.h"

#include <fstream>
#include <iostream>
#include <sstream>

namespace shealth {
namespace detail {
namespace csv {

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(line);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool loadFromCsv(const std::string& filename, std::vector<PersonRecord>& records, int maxRecords) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    records.clear();
    records.reserve(static_cast<std::size_t>(maxRecords));

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line)) {
        const std::vector<std::string> tokens = split(line, ',');
        if (tokens.empty()) {
            continue;
        }
        if (static_cast<int>(records.size()) >= maxRecords) {
            break;
        }
        if (tokens.size() < 4) {
            return false;
        }
        try {
            PersonRecord record;
            record.id = std::stoi(tokens[0]);
            record.age = std::stoi(tokens[1]);
            record.weight = std::stod(tokens[2]);
            record.height = std::stod(tokens[3]);
            records.push_back(record);
        } catch (const std::exception&) {
            return false;
        }
    }
    return true;
}

}  // namespace csv
}  // namespace detail
}  // namespace shealth
