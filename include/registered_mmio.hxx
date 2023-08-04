#pragma once

#include <any>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

/*
    The purpose of the MmioWrapper class is to provide an emulator agnostic way to hold a
    reference to an MMIO register, while also providing the possibility of describing what
    each bit in the register does. This is done by providing a binary string, where each
    character represents a bit in the register. The character 'x' means that the bit is
    used. The character '0' means that the bit is unused. The character ' is a separator
    for adjacent fields.

    TODO: make it so that 'x' makes the range appear as hex 'd' as decimal, 'b' as binary
    etc.
*/
namespace RegisteredMmio
{
    template <class ValueType>
    struct MmioWrapper_Impl
    {
        using type = ValueType;

        MmioWrapper_Impl(ValueType& value, std::string binary_string,
                         std::initializer_list<std::string> field_names)
            : value_(value), field_names_(field_names)
        {
            bool in_field = false;
            size_t field_end = 0;
            size_t final_string_size = 0;
            for (size_t i = 0; i < binary_string.size(); i++)
            {
                switch (binary_string[i])
                {
                    case 'x':
                    {
                        if (!in_field)
                        {
                            in_field = true;
                            field_end = sizeof(ValueType) * 8 - i - 1;
                        }
                        final_string_size++;
                        break;
                    }
                    case '\'':
                    {
                        if (in_field)
                        {
                            in_field = false;
                            field_bit_ranges_.emplace_back(sizeof(ValueType) * 8 - i, field_end);
                        }
                        break;
                    }
                    default:
                    {
                        if (in_field)
                        {
                            in_field = false;
                            field_bit_ranges_.emplace_back(sizeof(ValueType) * 8 - i, field_end);
                        }
                        final_string_size++;
                        break;
                    }
                }
            }

            if (final_string_size != sizeof(ValueType) * 8)
            {
                throw std::runtime_error("Binary string size does not match register size: " +
                                         binary_string);
            }

            if (in_field)
            {
                in_field = false;
                field_bit_ranges_.emplace_back(0, field_end);
            }

            if (field_bit_ranges_.size() != field_names_.size())
            {
                throw std::runtime_error("Number of fields does not match number of field names");
            }
        }

        ValueType GetRange(std::size_t range)
        {
            auto [start_bit, end_bit] = field_bit_ranges_[range];
            return (value_ >> start_bit) & ((1 << (end_bit - start_bit + 1)) - 1);
        }

        std::string GetFieldName(std::size_t range)
        {
            return field_names_[range];
        }

    private:
        ValueType& value_;
        std::vector<std::string> field_names_;
        std::vector<std::pair<std::size_t, std::size_t>> field_bit_ranges_;

        friend struct MmioWrapper;
    };

    struct MmioWrapper
    {
        MmioWrapper() = default;

        MmioWrapper(std::any mmio_wrapper, std::size_t size, std::string name)
            : mmio_wrapper_impl_(mmio_wrapper), size_(size), name_(name)
        {
            initialized_ = true;
        }

        uint64_t GetRange(std::size_t range)
        {
            if (!initialized_)
            {
                throw std::runtime_error("MmioWrapper not initialized during GetRange()");
            }

            switch (size_)
            {
                case 1:
                    return std::any_cast<MmioWrapper_Impl<uint8_t>>(mmio_wrapper_impl_)
                        .GetRange(range);
                case 2:
                    return std::any_cast<MmioWrapper_Impl<uint16_t>>(mmio_wrapper_impl_)
                        .GetRange(range);
                case 4:
                    return std::any_cast<MmioWrapper_Impl<uint32_t>>(mmio_wrapper_impl_)
                        .GetRange(range);
                case 8:
                    return std::any_cast<MmioWrapper_Impl<uint64_t>>(mmio_wrapper_impl_)
                        .GetRange(range);
                default:
                    throw std::runtime_error("Invalid register size");
            }
        }

        std::string GetFieldName(std::size_t range)
        {
            if (!initialized_)
            {
                throw std::runtime_error("MmioWrapper not initialized during GetFieldName()");
            }

            switch (size_)
            {
                case 1:
                    return std::any_cast<MmioWrapper_Impl<uint8_t>>(mmio_wrapper_impl_)
                        .GetFieldName(range);
                case 2:
                    return std::any_cast<MmioWrapper_Impl<uint16_t>>(mmio_wrapper_impl_)
                        .GetFieldName(range);
                case 4:
                    return std::any_cast<MmioWrapper_Impl<uint32_t>>(mmio_wrapper_impl_)
                        .GetFieldName(range);
                case 8:
                    return std::any_cast<MmioWrapper_Impl<uint64_t>>(mmio_wrapper_impl_)
                        .GetFieldName(range);
                default:
                    throw std::runtime_error("Invalid register size");
            }
        }

        std::size_t GetSize()
        {
            return size_;
        }

        std::string GetName()
        {
            return name_;
        }

        uint64_t GetValue()
        {
            if (!initialized_)
            {
                throw std::runtime_error("MmioWrapper not initialized during GetValue()");
            }

            switch (size_)
            {
                case 1:
                    return std::any_cast<MmioWrapper_Impl<uint8_t>>(mmio_wrapper_impl_).value_;
                case 2:
                    return std::any_cast<MmioWrapper_Impl<uint16_t>>(mmio_wrapper_impl_).value_;
                case 4:
                    return std::any_cast<MmioWrapper_Impl<uint32_t>>(mmio_wrapper_impl_).value_;
                case 8:
                    return std::any_cast<MmioWrapper_Impl<uint64_t>>(mmio_wrapper_impl_).value_;
                default:
                    throw std::runtime_error("Invalid register size");
            }
        }

        std::size_t GetRangeCount()
        {
            if (!initialized_)
            {
                throw std::runtime_error("MmioWrapper not initialized during GetRangeCount()");
            }

            switch (size_)
            {
                case 1:
                    return std::any_cast<MmioWrapper_Impl<uint8_t>>(mmio_wrapper_impl_)
                        .field_names_.size();
                case 2:
                    return std::any_cast<MmioWrapper_Impl<uint16_t>>(mmio_wrapper_impl_)
                        .field_names_.size();
                case 4:
                    return std::any_cast<MmioWrapper_Impl<uint32_t>>(mmio_wrapper_impl_)
                        .field_names_.size();
                case 8:
                    return std::any_cast<MmioWrapper_Impl<uint64_t>>(mmio_wrapper_impl_)
                        .field_names_.size();
                default:
                    throw std::runtime_error("Invalid register size");
            }
        }

    private:
        std::any mmio_wrapper_impl_;
        std::size_t size_;
        std::string name_;
        bool initialized_ = false;
    };

    template <class ValueType>
    MmioWrapper Register(std::string name, ValueType& value, std::string binary_string,
                         std::initializer_list<std::string> field_names)
    {
        return MmioWrapper(MmioWrapper_Impl(value, binary_string, field_names), sizeof(ValueType),
                           name);
    }

    using ComponentMmio = std::vector<MmioWrapper>;
    using ConsoleComponents = std::unordered_map<std::string, ComponentMmio>;
} // namespace RegisteredMmio