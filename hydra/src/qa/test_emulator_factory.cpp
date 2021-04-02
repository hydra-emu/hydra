#include <cppunit/extensions/HelperMacros.h>
#include <include/emulator_factory.h>

namespace TKPEmu::QA {
    class TestEmulatorFactory : public CppUnit::TestFixture {
        void testEmuTypes();
        CPPUNIT_TEST_SUITE(TestEmulatorFactory);
        CPPUNIT_TEST(testEmuTypes);
        CPPUNIT_TEST_SUITE_END();
    };
    void TestEmulatorFactory::testEmuTypes() {
        CPPUNIT_ASSERT_EQUAL(TKPEmu::EmuType::Gameboy, TKPEmu::EmulatorFactory::GetEmulatorType("type_detection.gb"));
        CPPUNIT_ASSERT_EQUAL(TKPEmu::EmuType::Gameboy, TKPEmu::EmulatorFactory::GetEmulatorType("type_detection.gbc"));
        CPPUNIT_ASSERT_EQUAL(TKPEmu::EmuType::NES, TKPEmu::EmulatorFactory::GetEmulatorType("type_detection.nes"));
        CPPUNIT_ASSERT_EQUAL(TKPEmu::EmuType::N64, TKPEmu::EmulatorFactory::GetEmulatorType("type_detection.z64"));
        CPPUNIT_ASSERT_EQUAL(TKPEmu::EmuType::Chip8, TKPEmu::EmulatorFactory::GetEmulatorType("type_detection.ch8"));
    }
    CPPUNIT_TEST_SUITE_REGISTRATION(TestEmulatorFactory);
}