#ifndef TKP_RESULTS_H
#define TKP_RESULTS_H
#include <unordered_map>
#include <string>
namespace TKPEmu::Testing {
    enum class TestResult { Unknown, Passed, Failed, None };
    // Used in the map below, to compare rom hashes with expected results after
    // a hardcoded number of clocks
    using Hash = std::string;
    struct ExpectedResult {
        int Clocks;
        // Represents the hash of the screenshot taken after Clocks
        Hash ExpectedHash;
        std::string TestName;
    };
    struct TestData {
        std::string RomName;
        TestResult Result;
    };
    // This map helps with quality assurance, we can check multiple test roms
    // at once and compare their finished hashes with these known good results
    struct QA {
        static std::unordered_map<Hash, ExpectedResult> PassedTestMap;
        static void Load(std::string path);
    };
}
#endif