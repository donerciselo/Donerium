#include "Themes.hpp"
#include "core/commands/ColorCommand.hpp"
#include "game/frontend/submenus/Settings/GUISettings.hpp"

namespace YimMenu
{
	void DefaultStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		// Text
		style.Colors[ImGuiCol_Text] = ImVec4(0.90f, 0.94f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.56f, 0.66f, 1.00f);

		// Backgrounds (frosted glass blue)
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.10f, 0.16f, 0.96f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.09f, 0.15f, 0.24f, 0.80f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.12f, 0.20f, 0.97f);

		// Borders (soft cyan-blue)
		style.Colors[ImGuiCol_Border] = ImVec4(0.29f, 0.58f, 0.87f, 0.40f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

		// Frames
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.20f, 0.32f, 1.00f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.42f, 0.66f, 0.60f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.31f, 0.56f, 0.84f, 0.70f);

		// Title bars
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.13f, 0.21f, 1.00f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.27f, 0.44f, 1.00f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.06f, 0.10f, 0.16f, 1.00f);

		// Menus/tabs
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.09f, 0.15f, 0.24f, 1.00f);
		style.Colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.38f, 0.60f, 0.85f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(0.29f, 0.52f, 0.78f, 0.85f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(0.31f, 0.56f, 0.84f, 1.00f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.28f, 0.45f, 0.70f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.38f, 0.60f, 0.85f);

		// Scrollbar
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.08f, 0.14f, 0.80f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.48f, 0.72f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.38f, 0.60f, 0.84f, 1.00f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.46f, 0.68f, 0.90f, 1.00f);

		// Check marks / sliders (bright cyan)
		style.Colors[ImGuiCol_CheckMark] = ImVec4(0.30f, 0.78f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.29f, 0.58f, 0.87f, 1.00f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.40f, 0.72f, 1.00f, 1.00f);

		// Buttons
		style.Colors[ImGuiCol_Button] = ImVec4(0.18f, 0.34f, 0.54f, 1.00f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.27f, 0.48f, 0.72f, 1.00f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.36f, 0.62f, 0.90f, 1.00f);

		// Headers
		style.Colors[ImGuiCol_Header] = ImVec4(0.21f, 0.39f, 0.60f, 1.00f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.29f, 0.52f, 0.76f, 1.00f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.36f, 0.62f, 0.90f, 1.00f);

		// Separators
		style.Colors[ImGuiCol_Separator] = ImVec4(0.45f, 0.62f, 0.80f, 0.50f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.55f, 0.80f, 1.00f, 0.78f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.65f, 0.85f, 1.00f, 1.00f);

		// Resize grips
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.50f, 0.70f, 0.90f, 0.30f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.60f, 0.80f, 1.00f, 0.60f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.70f, 0.85f, 1.00f, 0.90f);

		// Plots
		style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60f, 0.80f, 1.00f, 0.80f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.80f, 0.90f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.29f, 0.58f, 0.87f, 1.00f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.40f, 0.72f, 1.00f, 1.00f);

		// Drag & drop
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.40f, 0.80f, 1.00f, 0.90f);

		// Navigation highlighting
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.40f, 0.80f, 1.00f, 1.00f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.16f, 0.26f, 0.40f);

		// Modal/background dimming
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.09f, 0.15f, 0.50f);

		// Rounding (rounded glass look)
		style.GrabRounding = style.FrameRounding = style.ChildRounding = style.WindowRounding = 10.0f;
	}

	void SetupStyle()
	{
		// Apply default style first
		DefaultStyle();

		// Initialize the color/rounding commands and load saved settings
		InitializeColorCommands(); // This will call LoadSettings internally

		// Apply loaded colors/rounding to ImGui
		ApplyThemeToImGui();
	}
}
