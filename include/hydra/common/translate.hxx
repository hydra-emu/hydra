#pragma once

#include <fmt/format.h>

#include <QCoreApplication>
#include <QString>

template <>
struct fmt::formatter<QString> : fmt::formatter<std::string_view>
{
    auto format(const QString& str, format_context& ctx) const
    {
        return fmt::formatter<std::string_view>::format(str.toStdString(), ctx);
    }
};

template <typename... T>
QString htr(const char* fmt, T&&... args)
{
    std::string translated = QCoreApplication::translate("GlobalContext", fmt).toStdString();
    return QString::fromStdString(fmt::format(fmt::runtime(translated), std::forward<T>(args)...));
}