// Catalog of ImGui themes for the editor; call ApplyTheme once after ImGui::CreateContext.
#pragma once

namespace UIStyle {

    enum class Theme {
        ModelingDark,   // primary dark theme — optimised for 3D viewport work
        ModelingLight,  // high-brightness variant for bright-room use
    };

    void ApplyTheme(Theme theme);

}  // namespace UIStyle
