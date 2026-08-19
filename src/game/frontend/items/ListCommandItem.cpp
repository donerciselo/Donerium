#include "Items.hpp"
#include "core/commands/Command.hpp"
#include "core/commands/Commands.hpp"
#include "core/commands/ListCommand.hpp"
#include "core/frontend/widgets/toggle/imgui_toggle.hpp"
#include "core/util/Localization.hpp"

namespace YimMenu
{
	ListCommandItem::ListCommandItem(joaat_t id, std::optional<std::string> label_override) :
	    m_Command(Commands::GetCommand<ListCommand>(id)),
	    m_LabelOverride(label_override.has_value() ? std::optional<std::string>(tr(label_override.value())) : std::nullopt)
	{
	}

	void ListCommandItem::Draw()
	{
		if (!m_Command)
		{
			ImGui::Text("Bilinmeyen liste!");
			return;
		}

		int current_val = m_Command->GetState();
		auto& list = m_Command->GetList();
		std::string largest_translated;
		std::size_t largest_string_len = 0;

		if (!m_SelectedItem.has_value() || !m_ItemWidth.has_value())
		{
			for (auto& item : list)
			{
				if (item.first == current_val)
				{
					m_SelectedItem = tr(item.second);
				}

				auto translated = tr(item.second);
				int length = strlen(translated.c_str());
				if (length > largest_string_len)
				{
					largest_translated = translated;
					largest_string_len = length;
				}
			}

			if (!m_SelectedItem.has_value())
				m_SelectedItem = "";

			auto size = ImGui::CalcTextSize(largest_translated.c_str());
			m_ItemWidth = size.x + 40.0f;
		}

		ImGui::SetNextItemWidth(m_ItemWidth.value());
		if (ImGui::BeginCombo(m_LabelOverride.value_or(m_Command->GetLabel()).c_str(), m_SelectedItem.value().c_str()))
		{
			for (auto& el : list)
			{
				auto translated = tr(el.second);
				if (ImGui::Selectable(translated.c_str(), el.first == current_val))
				{
					current_val = el.first;
					m_Command->SetState(el.first);
				}

				if (el.first == current_val)
				{
					m_SelectedItem = translated; // just in case
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}
}