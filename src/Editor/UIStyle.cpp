#include "Editor/UIStyle.hpp"
#include "Renderer/Renderer.hpp"
#include "imgui.h"

namespace UIStyle {
    namespace {

        // ─── helpers ────────────────────────────────────────────────────────────────

        constexpr ImVec4 rgba(float r, float g, float b, float a = 1.0f) {
            return {r, g, b, a};
        }

        // ─── shared shape / layout tweaks (same for all themes) ─────────────────────

        void applyShapeDefaults(ImGuiStyle& s) {
            s.WindowRounding    = 4.0f;
            s.ChildRounding     = 4.0f;
            s.FrameRounding     = 3.0f;
            s.PopupRounding     = 3.0f;
            s.ScrollbarRounding = 3.0f;
            s.GrabRounding      = 3.0f;
            s.TabRounding       = 4.0f;

            s.WindowBorderSize = 1.0f;
            s.FrameBorderSize  = 0.0f;
            s.PopupBorderSize  = 1.0f;

            s.WindowPadding    = {8.0f, 8.0f};
            s.FramePadding     = {6.0f, 3.0f};
            s.ItemSpacing      = {6.0f, 4.0f};
            s.ItemInnerSpacing = {4.0f, 4.0f};
            s.IndentSpacing    = 14.0f;
            s.ScrollbarSize    = 11.0f;
            s.GrabMinSize      = 8.0f;
        }

        // ─── ModelingDark ────────────────────────────────────────────────────────────
        //
        // Palette reference
        //   bg-deep   #111111   panels, sidebar
        //   bg-mid    #1A1A1A   window backgrounds
        //   bg-raised #222222   frames, inputs, table rows
        //   bg-hover  #2D2D2D   hover highlight
        //   border    #3A3A3A   subtle separators
        //   accent    #4A8FD4   primary interaction (blue, low saturation)
        //   accent2   #D4884A   secondary / warning (orange, matches viewport selection)
        //   text-hi   #E8E8E8   primary labels
        //   text-dim  #888888   disabled / secondary labels

        void applyModelingDark() {
            ImGui::StyleColorsDark();
            ImGuiStyle& s = ImGui::GetStyle();
            applyShapeDefaults(s);

            auto* c = s.Colors;

            // ── Chrome ──
            c[ImGuiCol_WindowBg]     = rgba(0.10f, 0.10f, 0.10f);
            c[ImGuiCol_ChildBg]      = rgba(0.08f, 0.08f, 0.08f);
            c[ImGuiCol_PopupBg]      = rgba(0.12f, 0.12f, 0.12f, 0.97f);
            c[ImGuiCol_Border]       = rgba(0.23f, 0.23f, 0.23f, 0.80f);
            c[ImGuiCol_BorderShadow] = rgba(0.00f, 0.00f, 0.00f, 0.00f);

            // ── Title bars ──
            c[ImGuiCol_TitleBg]          = rgba(0.07f, 0.07f, 0.07f);
            c[ImGuiCol_TitleBgActive]    = rgba(0.13f, 0.13f, 0.13f);
            c[ImGuiCol_TitleBgCollapsed] = rgba(0.07f, 0.07f, 0.07f, 0.75f);

            // ── Menu bar ──
            c[ImGuiCol_MenuBarBg] = rgba(0.07f, 0.07f, 0.07f);

            // ── Frames (inputs, combo boxes, sliders…) ──
            c[ImGuiCol_FrameBg]        = rgba(0.14f, 0.14f, 0.14f);
            c[ImGuiCol_FrameBgHovered] = rgba(0.20f, 0.20f, 0.20f);
            c[ImGuiCol_FrameBgActive]  = rgba(0.25f, 0.25f, 0.25f);

            // ── Scrollbar ──
            c[ImGuiCol_ScrollbarBg]          = rgba(0.08f, 0.08f, 0.08f);
            c[ImGuiCol_ScrollbarGrab]        = rgba(0.30f, 0.30f, 0.30f);
            c[ImGuiCol_ScrollbarGrabHovered] = rgba(0.40f, 0.40f, 0.40f);
            c[ImGuiCol_ScrollbarGrabActive]  = rgba(0.50f, 0.50f, 0.50f);

            // ── Accent (blue) — used for checks, sliders, active selection ──
            c[ImGuiCol_CheckMark]        = rgba(0.29f, 0.56f, 0.84f);
            c[ImGuiCol_SliderGrab]       = rgba(0.29f, 0.56f, 0.84f, 0.90f);
            c[ImGuiCol_SliderGrabActive] = rgba(0.40f, 0.68f, 1.00f);

            // ── Buttons ──
            c[ImGuiCol_Button]        = rgba(0.20f, 0.20f, 0.20f);
            c[ImGuiCol_ButtonHovered] = rgba(0.29f, 0.56f, 0.84f, 0.85f);
            c[ImGuiCol_ButtonActive]  = rgba(0.22f, 0.45f, 0.72f);

            // ── Tree / collapsible headers ──
            c[ImGuiCol_Header]        = rgba(0.19f, 0.37f, 0.60f, 0.70f);
            c[ImGuiCol_HeaderHovered] = rgba(0.29f, 0.56f, 0.84f, 0.80f);
            c[ImGuiCol_HeaderActive]  = rgba(0.29f, 0.56f, 0.84f);

            // ── Separators ──
            c[ImGuiCol_Separator]        = rgba(0.23f, 0.23f, 0.23f);
            c[ImGuiCol_SeparatorHovered] = rgba(0.29f, 0.56f, 0.84f, 0.70f);
            c[ImGuiCol_SeparatorActive]  = rgba(0.29f, 0.56f, 0.84f);

            // ── Resize grips ──
            c[ImGuiCol_ResizeGrip]        = rgba(0.29f, 0.56f, 0.84f, 0.20f);
            c[ImGuiCol_ResizeGripHovered] = rgba(0.29f, 0.56f, 0.84f, 0.60f);
            c[ImGuiCol_ResizeGripActive]  = rgba(0.29f, 0.56f, 0.84f, 0.95f);

            // ── Tabs ──
            c[ImGuiCol_Tab]                 = rgba(0.12f, 0.12f, 0.12f);
            c[ImGuiCol_TabHovered]          = rgba(0.29f, 0.56f, 0.84f, 0.80f);
            c[ImGuiCol_TabSelected]         = rgba(0.19f, 0.37f, 0.60f);
            c[ImGuiCol_TabSelectedOverline] = rgba(0.29f, 0.56f, 0.84f);
            c[ImGuiCol_TabDimmed]           = rgba(0.09f, 0.09f, 0.09f);
            c[ImGuiCol_TabDimmedSelected]   = rgba(0.14f, 0.26f, 0.42f);

            // ── Plot / progress ──
            c[ImGuiCol_PlotLines]            = rgba(0.56f, 0.56f, 0.56f);
            c[ImGuiCol_PlotLinesHovered]     = rgba(0.83f, 0.54f, 0.29f);  // orange accent
            c[ImGuiCol_PlotHistogram]        = rgba(0.29f, 0.56f, 0.84f, 0.80f);
            c[ImGuiCol_PlotHistogramHovered] = rgba(0.40f, 0.68f, 1.00f);

            // ── Tables ──
            c[ImGuiCol_TableHeaderBg]     = rgba(0.13f, 0.13f, 0.13f);
            c[ImGuiCol_TableBorderStrong] = rgba(0.23f, 0.23f, 0.23f);
            c[ImGuiCol_TableBorderLight]  = rgba(0.16f, 0.16f, 0.16f);
            c[ImGuiCol_TableRowBg]        = rgba(0.00f, 0.00f, 0.00f, 0.00f);
            c[ImGuiCol_TableRowBgAlt]     = rgba(1.00f, 1.00f, 1.00f, 0.03f);

            // ── Text ──
            c[ImGuiCol_Text]           = rgba(0.91f, 0.91f, 0.91f);
            c[ImGuiCol_TextDisabled]   = rgba(0.50f, 0.50f, 0.50f);
            c[ImGuiCol_TextSelectedBg] = rgba(0.29f, 0.56f, 0.84f, 0.35f);

            // ── Misc ──
            c[ImGuiCol_DragDropTarget]        = rgba(0.83f, 0.54f, 0.29f);  // orange accent
            c[ImGuiCol_NavHighlight]          = rgba(0.29f, 0.56f, 0.84f);
            c[ImGuiCol_NavWindowingHighlight] = rgba(1.00f, 1.00f, 1.00f, 0.70f);
            c[ImGuiCol_NavWindowingDimBg]     = rgba(0.80f, 0.80f, 0.80f, 0.20f);
            c[ImGuiCol_ModalWindowDimBg]      = rgba(0.10f, 0.10f, 0.10f, 0.55f);
        }

        // ─── ModelingLight ───────────────────────────────────────────────────────────

        void applyModelingLight() {
            ImGui::StyleColorsLight();
            ImGuiStyle& s = ImGui::GetStyle();
            applyShapeDefaults(s);

            auto* c = s.Colors;

            c[ImGuiCol_WindowBg] = rgba(0.88f, 0.88f, 0.88f);
            c[ImGuiCol_ChildBg]  = rgba(0.84f, 0.84f, 0.84f);
            c[ImGuiCol_PopupBg]  = rgba(0.92f, 0.92f, 0.92f, 0.97f);
            c[ImGuiCol_Border]   = rgba(0.60f, 0.60f, 0.60f, 0.80f);

            c[ImGuiCol_TitleBg]       = rgba(0.76f, 0.76f, 0.76f);
            c[ImGuiCol_TitleBgActive] = rgba(0.70f, 0.70f, 0.70f);
            c[ImGuiCol_MenuBarBg]     = rgba(0.76f, 0.76f, 0.76f);

            c[ImGuiCol_FrameBg]        = rgba(0.78f, 0.78f, 0.78f);
            c[ImGuiCol_FrameBgHovered] = rgba(0.72f, 0.72f, 0.72f);
            c[ImGuiCol_FrameBgActive]  = rgba(0.66f, 0.66f, 0.66f);

            // blue accent
            c[ImGuiCol_CheckMark]        = rgba(0.16f, 0.44f, 0.75f);
            c[ImGuiCol_SliderGrab]       = rgba(0.16f, 0.44f, 0.75f, 0.90f);
            c[ImGuiCol_SliderGrabActive] = rgba(0.10f, 0.34f, 0.65f);

            c[ImGuiCol_Button]        = rgba(0.72f, 0.72f, 0.72f);
            c[ImGuiCol_ButtonHovered] = rgba(0.16f, 0.44f, 0.75f, 0.80f);
            c[ImGuiCol_ButtonActive]  = rgba(0.10f, 0.34f, 0.65f);

            c[ImGuiCol_Header]        = rgba(0.16f, 0.44f, 0.75f, 0.50f);
            c[ImGuiCol_HeaderHovered] = rgba(0.16f, 0.44f, 0.75f, 0.70f);
            c[ImGuiCol_HeaderActive]  = rgba(0.16f, 0.44f, 0.75f);

            c[ImGuiCol_Tab]         = rgba(0.80f, 0.80f, 0.80f);
            c[ImGuiCol_TabHovered]  = rgba(0.16f, 0.44f, 0.75f, 0.70f);
            c[ImGuiCol_TabSelected] = rgba(0.16f, 0.44f, 0.75f, 0.90f);

            c[ImGuiCol_Text]           = rgba(0.10f, 0.10f, 0.10f);
            c[ImGuiCol_TextDisabled]   = rgba(0.50f, 0.50f, 0.50f);
            c[ImGuiCol_TextSelectedBg] = rgba(0.16f, 0.44f, 0.75f, 0.30f);
        }

    }  // namespace

    // ─── dispatch ────────────────────────────────────────────────────────────────

    void ApplyTheme(Theme theme) {
        switch (theme) {
            case Theme::ModelingDark:
                applyModelingDark();
                Renderer::SetSceneClearColor({0.1f, 0.1f, 0.1f, 1.0f});
                break;
            case Theme::ModelingLight:
                applyModelingLight();
                Renderer::SetSceneClearColor({0.55f, 0.55f, 0.55f, 1.0f});
                break;
        }
    }

}  // namespace UIStyle
