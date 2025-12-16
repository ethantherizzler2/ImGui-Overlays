#include "notification.h"
#include "dependencies/imgui/imgui.h"
#include <vector>

struct Notification
{
    std::string title;
    std::string message;
    Notify::Type type;

    float lifetime;
    float timer;

    float slide; // 0.0 -> 1.0
};

static std::vector<Notification> notifications;

void Notify::Push(const std::string& title,
    const std::string& message,
    Type type,
    float duration)
{
    notifications.push_back({
        title,
        message,
        type,
        duration,
        0.0f,
        0.0f
        });
}

static ImU32 GetTypeColor(Notify::Type type)
{
    switch (type)
    {
    case Notify::Type::Info:    return IM_COL32(200, 200, 200, 255);
    case Notify::Type::Warning: return IM_COL32(220, 220, 220, 255);
    case Notify::Type::Error:   return IM_COL32(255, 255, 255, 255);
    default:                    return IM_COL32(255, 255, 255, 255);
    }
}

void Notify::Render()
{
    if (notifications.empty())
        return;

    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw = ImGui::GetForegroundDrawList();

    const float width = 320.f;
    const float height = 70.f;
    const float padding = 15.f;
    const float slideSpeed = 6.f;

    float yOffset = 0.f;

    for (int i = 0; i < notifications.size();)
    {
        auto& n = notifications[i];

        n.timer += io.DeltaTime;

        // slide in
        n.slide += io.DeltaTime * slideSpeed;
        if (n.slide > 1.f)
            n.slide = 1.f;

        // slide out
        if (n.timer > n.lifetime)
            n.slide -= io.DeltaTime * slideSpeed;

        if (n.slide <= 0.f)
        {
            notifications.erase(notifications.begin() + i);
            continue;
        }

        float slideX = (1.f - n.slide) * (width + 20.f);

        ImVec2 pos(
            io.DisplaySize.x - width - padding + slideX,
            io.DisplaySize.y - height - padding - yOffset
        );

        ImVec2 size(width, height);

        // background
        draw->AddRectFilled(
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            IM_COL32(0, 0, 0, 220),
            6.f
        );

        // border
        draw->AddRect(
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            IM_COL32(255, 255, 255, 60),
            6.f
        );

        // title
        draw->AddText(
            ImVec2(pos.x + 12, pos.y + 8),
            GetTypeColor(n.type),
            n.title.c_str()
        );

        // message
        draw->AddText(
            ImVec2(pos.x + 12, pos.y + 30),
            IM_COL32(200, 200, 200, 255),
            n.message.c_str()
        );

        yOffset += height + 10.f;
        ++i;
    }
}
