#pragma once
#include <string>

namespace Notify
{
    enum class Type
    {
        Info,
        Warning,
        Error
    };

    void Push(const std::string& title,
        const std::string& message,
        Type type = Type::Info,
        float duration = 3.0f);

    void Render();
}
