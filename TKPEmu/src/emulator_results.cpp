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
            rom_hash.resize(32);
            res.ExpectedHash.resize(32);
            res.TestName.resize(100);
            sscanf(line.c_str(), "%s,%u,%s,%s", rom_hash.data(), &res.Clocks, res.ExpectedHash.data(), res.TestName.data());
            std::cout << rom_hash << " " << res.Clocks << " " << res.ExpectedHash << " " << res.TestName << std::endl;
            PassedTestMap.emplace(rom_hash, res);
        }
    }
}