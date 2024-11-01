#pragma once

#include <AalTest/API.h>
#include <chrono>
#include <QString>
#include <tuple>

namespace AalTest
{
    AALTEST_API QString Stringify(bool value);
    AALTEST_API QString Stringify(int value);
    AALTEST_API QString Stringify(long long value);
    AALTEST_API QString Stringify(const char* str);
    AALTEST_API QString Stringify(const QString& string);
    AALTEST_API QString Stringify(const QStringView stringView);
    AALTEST_API QString Stringify(const std::chrono::nanoseconds& ns);

    template<class T>
    QString Stringify(T t)
    {
        return QString("?");
    }

    template<class ...Ts>
    QString Stringify(const std::tuple<Ts...>& tuple)
    {
        auto converter = [](const auto& ...args) { return QStringList{ Stringify(args)... }; };
        auto parts = std::apply(converter, tuple);
        return QString("(%1)").arg(parts.join(", "));
    }

    AALTEST_API QString Sanitize(QString input);
}
