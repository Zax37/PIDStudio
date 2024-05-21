#pragma once

#include <SFML/Graphics/Texture.hpp>

class ImGuiWindow;

namespace ImGui {
    void CenteredImage(const sf::Texture& texture);
    bool BringFocusTo(ImGuiWindow* window);
    bool BeginPopupForLastItem(const char* name);
}
