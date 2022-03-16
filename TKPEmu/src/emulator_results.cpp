#include "../include/emulator_results.h"
#include <fstream>
#include <iostream>

namespace TKPEmu::Testing {
    std::unordered_map<Hash, ExpectedResult> QA::PassedTestMap;
    void QA::Load(std::string path) {
        std::ifstream ifs(path, std::ios::in);
        std::string line;
        while (std::getline(ifs, line)) {
            Hash rom_hash;
            ExpectedResult res;
            rom_hash = line.substr(0, 32);
            auto temp = line.substr(32 + 1);
            size_t next_comma = temp.find(',');
            auto num = temp.substr(0, next_comma);
            res.Clocks = std::stoi(num);
            auto rest = temp.substr(next_comma + 1);
            res.ExpectedHash = rest.substr(0, 32);
            res.TestName = rest.substr(32 + 1); 
            PassedTestMap.emplace(rom_hash, res);
        }
    }
}