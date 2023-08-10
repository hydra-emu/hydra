#pragma once

#include <n64/core/n64_types.hxx>

namespace hydra::N64
{
    enum class RSPHWIO {
        Cache = 0,
        DramAddr = 1,
        RdLen = 2,
        WrLen = 3,
        Status = 4,
        Full = 5,
        Busy = 6,
        Semaphore = 7,
        CmdStart = 8,
        CmdEnd = 9,
        CmdCurrent = 10,
        CmdStatus = 11,
        CmdClock = 12,
        CmdBusy = 13,
        CmdPipeBusy = 14,
        CmdTmemBusy = 15,
    };

    class CPU;
    class CPUBus;
    class RCP;
    class RSP;
    class RDP;
    using VectorRegister = std::array<uint16_t, 8>;

    struct AccumulatorLane
    {
        void Set(uint64_t new_value)
        {
            value_ = new_value & 0xFFFF'FFFF'FFFF;
        }

        void SetHigh(uint16_t new_value)
        {
            value_ = (static_cast<uint64_t>(new_value) << 32) | (value_ & 0xFFFF'FFFF);
        }

        void SetMiddle(uint16_t new_value)
        {
            value_ = (static_cast<uint64_t>(new_value) << 16) | (value_ & 0xFFFF'0000'FFFF);
        }

        void SetLow(uint16_t new_value)
        {
            value_ = (value_ & 0xFFFF'FFFF'0000) | new_value;
        }

        uint64_t Get() const
        {
            return value_;
        }

        int64_t GetSigned() const
        {
            return static_cast<int64_t>(value_ << 16) >> 16;
        }

        uint16_t GetHigh() const
        {
            return value_ >> 32;
        }

        uint16_t GetMiddle() const
        {
            return (value_ >> 16) & 0xFFFF;
        }

        uint16_t GetLow() const
        {
            return value_ & 0xFFFF;
        }

        int16_t GetHighSigned() const
        {
            return value_ >> 32;
        }

        void Add(int64_t value)
        {
            value_ = static_cast<int64_t>(value_) + value;
            value_ &= 0xFFFF'FFFF'FFFF;
        }

    private:
        uint64_t value_ = 0;
    };

    struct VUControl16
    {
        bool GetLow(int index) const
        {
            return (full_ >> index) & 1;
        }

        bool GetHigh(int index) const
        {
            return (full_ >> (index + 8)) & 1;
        }

        void SetLow(int index, bool value)
        {
            full_ = (full_ & ~(1 << index)) | (value << index);
        }

        void SetHigh(int index, bool value)
        {
            full_ = (full_ & ~(1 << (index + 8))) | (value << (index + 8));
        }

        void Clear()
        {
            full_ = 0;
        }

        uint16_t& operator*()
        {
            return full_;
        }

    private:
        uint16_t full_ = 0;
    };

    struct VUControl8
    {
        bool Get(int index) const
        {
            return (full_ >> index) & 1;
        }

        void Set(int index, bool value)
        {
            full_ = (full_ & ~(1 << index)) | (value << index);
        }

        void Clear()
        {
            full_ = 0;
        }

        uint8_t& operator*()
        {
            return full_;
        }

    private:
        uint8_t full_ = 0;
    };

    union RSPStatus
    {
        uint32_t full;

        struct
        {
            uint32_t halt       : 1;
            uint32_t broke      : 1;
            uint32_t dma_busy   : 1;
            uint32_t dma_full   : 1;
            uint32_t io_busy    : 1;
            uint32_t sstep      : 1;
            uint32_t intr_break : 1;
            uint32_t signal_0   : 1;
            uint32_t signal_1   : 1;
            uint32_t signal_2   : 1;
            uint32_t signal_3   : 1;
            uint32_t signal_4   : 1;
            uint32_t signal_5   : 1;
            uint32_t signal_6   : 1;
            uint32_t signal_7   : 1;
            uint32_t            : 17;
        };
    };

    static_assert(sizeof(RSPStatus) == sizeof(uint32_t));

    union RSPStatusWrite
    {
        uint32_t full;

        struct
        {
            uint32_t clear_halt       : 1;
            uint32_t set_halt         : 1;
            uint32_t clear_broke      : 1;
            uint32_t clear_intr       : 1;
            uint32_t set_intr         : 1;
            uint32_t clear_sstep      : 1;
            uint32_t set_sstep        : 1;
            uint32_t clear_intr_break : 1;
            uint32_t set_intr_break   : 1;
            uint32_t clear_signal_0   : 1;
            uint32_t set_signal_0     : 1;
            uint32_t clear_signal_1   : 1;
            uint32_t set_signal_1     : 1;
            uint32_t clear_signal_2   : 1;
            uint32_t set_signal_2     : 1;
            uint32_t clear_signal_3   : 1;
            uint32_t set_signal_3     : 1;
            uint32_t clear_signal_4   : 1;
            uint32_t set_signal_4     : 1;
            uint32_t clear_signal_5   : 1;
            uint32_t set_signal_5     : 1;
            uint32_t clear_signal_6   : 1;
            uint32_t set_signal_6     : 1;
            uint32_t clear_signal_7   : 1;
            uint32_t set_signal_7     : 1;
            uint32_t                  : 7;
        };
    };

    static_assert(sizeof(RSPStatusWrite) == sizeof(uint32_t));

    template <auto MemberFunc>
    static void lut_wrapper(RSP* cpu)
    {
        (cpu->*MemberFunc)();
    }

    class RSP final
    {
    public:
        RSP();
        void Tick();
        void Reset();

        bool IsHalted()
        {
            return status_.halt;
        }

        void InstallBuses(uint8_t* rdram_ptr, RDP* rdp_ptr)
        {
            rdram_ptr_ = rdram_ptr;
            rdp_ptr_ = rdp_ptr;
        }

        void SetMIPtr(MIInterrupt* ptr)
        {
            mi_interrupt_ = ptr;
        }

    private:
        using func_ptr = void (*)(RSP*);

        void VMULF(), VMULU(), VMUDL(), VMUDM(), VMUDN(), VMUDH(), VMACF(), VMACU(), VMADL(),
            VMADM(), VMADN(), VMADH(), VADD(), VABS(), VADDC(), VSAR(), VAND(), VNAND(), VOR(),
            VNOR(), VXOR(), VNXOR(), VSUB(), VLT(), VSUBC(), VEQ(), VNE(), VGE(), VCL(), VCH(),
            VCR(), VMRG(), VRCP(), VRCPL(), VRCPH(), VMOV(), VRSQ(), VRSQL(), VMULQ(), VMACQ(),
            VZERO(), VNOP();

        void SPECIAL(), REGIMM(), J(), JAL(), BEQ(), BNE(), BLEZ(), BGTZ(), ADDI(), ADDIU(), SLTI(),
            SLTIU(), ANDI(), ORI(), XORI(), LUI(), COP0(), COP1(), COP2(), LB(), LH(), LW(), LBU(),
            LHU(), LWU(), SB(), SH(), SW(), CACHE(), LWC2(), SWC2();

        void s_SLL(), s_SRL(), s_SRA(), s_SLLV(), s_SRLV(), s_SRAV(), s_JR(), s_JALR(), s_ADDU(),
            s_SUBU(), s_SLTU(), s_SLT(), s_AND(), s_OR(), s_XOR(), s_NOR(), s_BREAK();

        void r_BLTZ(), r_BGEZ(), r_BLTZAL(), r_BGEZAL();

        void SBV(), SSV(), SLV(), SDV(), SQV(), SRV(), SPV(), SUV(), SHV(), SFV(), SWV(), STV();
        void LBV(), LSV(), LLV(), LDV(), LQV(), LRV(), LPV(), LUV(), LHV(), LFV(), LWV(), LTV();

        void MFC2(), CFC2(), MTC2(), CTC2();

        void ERROR();
        void ERROR2();

        constexpr static std::array<func_ptr, 64> instruction_table_ = {
            &lut_wrapper<&RSP::SPECIAL>, &lut_wrapper<&RSP::REGIMM>, &lut_wrapper<&RSP::J>,
            &lut_wrapper<&RSP::JAL>,     &lut_wrapper<&RSP::BEQ>,    &lut_wrapper<&RSP::BNE>,
            &lut_wrapper<&RSP::BLEZ>,    &lut_wrapper<&RSP::BGTZ>,   &lut_wrapper<&RSP::ADDI>,
            &lut_wrapper<&RSP::ADDIU>,   &lut_wrapper<&RSP::SLTI>,   &lut_wrapper<&RSP::SLTIU>,
            &lut_wrapper<&RSP::ANDI>,    &lut_wrapper<&RSP::ORI>,    &lut_wrapper<&RSP::XORI>,
            &lut_wrapper<&RSP::LUI>,     &lut_wrapper<&RSP::COP0>,   &lut_wrapper<&RSP::COP1>,
            &lut_wrapper<&RSP::COP2>,    &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::LB>,
            &lut_wrapper<&RSP::LH>,      &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::LW>,
            &lut_wrapper<&RSP::LBU>,     &lut_wrapper<&RSP::LHU>,    &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::LWU>,     &lut_wrapper<&RSP::SB>,     &lut_wrapper<&RSP::SH>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::SW>,     &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::CACHE>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::LWC2>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::SWC2>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,
        };

        constexpr static std::array<func_ptr, 64> special_table_ = {
            &lut_wrapper<&RSP::s_SLL>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::s_SRL>,
            &lut_wrapper<&RSP::s_SRA>,  &lut_wrapper<&RSP::s_SLLV>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::s_SRLV>, &lut_wrapper<&RSP::s_SRAV>,  &lut_wrapper<&RSP::s_JR>,
            &lut_wrapper<&RSP::s_JALR>, &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::s_BREAK>, &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::s_ADDU>,
            &lut_wrapper<&RSP::s_ADDU>, &lut_wrapper<&RSP::s_SUBU>,  &lut_wrapper<&RSP::s_SUBU>,
            &lut_wrapper<&RSP::s_AND>,  &lut_wrapper<&RSP::s_OR>,    &lut_wrapper<&RSP::s_XOR>,
            &lut_wrapper<&RSP::s_NOR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::s_SLT>,  &lut_wrapper<&RSP::s_SLTU>,  &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,  &lut_wrapper<&RSP::ERROR>,   &lut_wrapper<&RSP::ERROR>,
            &lut_wrapper<&RSP::ERROR>,
        };

        constexpr static std::array<func_ptr, 64> vu_instruction_table_ = {
            &lut_wrapper<&RSP::VMULF>, &lut_wrapper<&RSP::VMULU>, &lut_wrapper<&RSP::VZERO>,
            &lut_wrapper<&RSP::VMULQ>, &lut_wrapper<&RSP::VMUDL>, &lut_wrapper<&RSP::VMUDM>,
            &lut_wrapper<&RSP::VMUDN>, &lut_wrapper<&RSP::VMUDH>, &lut_wrapper<&RSP::VMACF>,
            &lut_wrapper<&RSP::VMACU>, &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>,
            &lut_wrapper<&RSP::VMADL>, &lut_wrapper<&RSP::VMADM>, &lut_wrapper<&RSP::VMADN>,
            &lut_wrapper<&RSP::VMADH>, &lut_wrapper<&RSP::VADD>,  &lut_wrapper<&RSP::VSUB>,
            &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VABS>,  &lut_wrapper<&RSP::VADDC>,
            &lut_wrapper<&RSP::VSUBC>, &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>,
            &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>,
            &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VSAR>,
            &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VLT>,
            &lut_wrapper<&RSP::VEQ>,   &lut_wrapper<&RSP::VNE>,   &lut_wrapper<&RSP::VGE>,
            &lut_wrapper<&RSP::VCL>,   &lut_wrapper<&RSP::VCH>,   &lut_wrapper<&RSP::VCR>,
            &lut_wrapper<&RSP::VMRG>,  &lut_wrapper<&RSP::VAND>,  &lut_wrapper<&RSP::VNAND>,
            &lut_wrapper<&RSP::VOR>,   &lut_wrapper<&RSP::VNOR>,  &lut_wrapper<&RSP::VXOR>,
            &lut_wrapper<&RSP::VNXOR>, &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>,
            &lut_wrapper<&RSP::VRCP>,  &lut_wrapper<&RSP::VRCPL>, &lut_wrapper<&RSP::VRCPH>,
            &lut_wrapper<&RSP::VMOV>,  &lut_wrapper<&RSP::VRSQ>,  &lut_wrapper<&RSP::VRSQL>,
            &lut_wrapper<&RSP::VRCPH>, &lut_wrapper<&RSP::VNOP>,  &lut_wrapper<&RSP::VZERO>,
            &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>,
            &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>, &lut_wrapper<&RSP::VZERO>,
            &lut_wrapper<&RSP::VNOP>,
        };

        constexpr static std::array<func_ptr, 32> regimm_table_ = {
            &lut_wrapper<&RSP::r_BLTZ>, &lut_wrapper<&RSP::r_BGEZ>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::r_BLTZAL>, &lut_wrapper<&RSP::r_BGEZAL>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,   &lut_wrapper<&RSP::ERROR2>,
            &lut_wrapper<&RSP::ERROR2>, &lut_wrapper<&RSP::ERROR2>,
        };

        void execute_instruction();
        uint32_t fetch_instruction();
        uint8_t load_byte(uint16_t address);
        uint16_t load_halfword(uint16_t address);
        uint32_t load_word(uint16_t address);
        void store_byte(uint16_t address, uint8_t value);
        void store_halfword(uint16_t address, uint16_t value);
        void store_word(uint16_t address, uint32_t value);
        void branch_to(uint16_t address);
        void conditional_branch(bool condition, uint16_t address);
        void link_register(uint8_t reg);
        void read_dma();
        void write_dma();
        void dump_mem();
        int16_t get_lane(int reg, int lane);
        void set_lane(int reg, int lane, int16_t value);
        int16_t get_control(int reg);
        void set_control(int reg, int16_t value);
        void write_hwio(RSPHWIO addr, uint32_t data);
        uint32_t read_hwio(RSPHWIO addr);

        VectorRegister& get_vs();
        VectorRegister& get_vt();
        VectorRegister& get_vd();

        template <bool DoLog>
        void log_cpu_state(bool use_crc, uint64_t instructions);

        std::array<uint8_t, 0x2000> mem_{};
        std::array<MemDataUnionW, 32> gpr_regs_;
        std::array<VectorRegister, 32> vu_regs_;
        VUControl16 vco_, vcc_;
        VUControl8 vce_;
        int16_t div_in_, div_out_;
        bool div_in_ready_;
        std::array<AccumulatorLane, 8> accumulator_;

        // TODO: some are probably not needed
        Instruction instruction_;
        uint32_t mem_addr_;
        bool dma_imem_;
        uint32_t rdram_addr_;
        uint32_t rd_len_;
        uint32_t wr_len_;
        RSPStatus status_;
        uint32_t pc_;
        uint32_t next_pc_;
        bool semaphore_;
        uint8_t* rdram_ptr_ = nullptr;
        MIInterrupt* mi_interrupt_ = nullptr;
        RDP* rdp_ptr_ = nullptr;

        friend class hydra::N64::CPU;
        friend class hydra::N64::CPUBus;
        friend class hydra::N64::RCP;
        friend class MmioViewer;
    };
} // namespace hydra::N64