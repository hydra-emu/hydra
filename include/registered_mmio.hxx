#pragma once

#include <any>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace RegisteredMmio
{

    static std::vector<std::string> registered_mmio;

    template <class ValueType>
    struct MmioWrapper_Impl
    {
        using type = ValueType;

        MmioWrapper_Impl(ValueType& value, std::string binary_string,
                         std::initializer_list<std::string> field_names)
            : value_(value), field_names_(field_names)
        {
            if (binary_string.size() != sizeof(ValueType) * 8)
            {
                throw std::runtime_error("Binary string size does not match register size: " +
                                         binary_string);
            }

            bool in_field = false;
            std::size_t field_end = 0;
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
                        break;
                    }
                    default:
                    {
                        if (in_field)
                        {
                            in_field = false;
                            field_bit_ranges_.emplace_back(sizeof(ValueType) * 8 - i, field_end);
                        }
                        break;
                    }
                }
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
                throw std::runtime_error("MmioWrapper not initialized");
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
                throw std::runtime_error("MmioWrapper not initialized");
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

    using ComponentMmio = std::unordered_map<std::string, MmioWrapper>;
    using ConsoleComponents = std::unordered_map<std::string, ComponentMmio>;
} // namespace RegisteredMmio