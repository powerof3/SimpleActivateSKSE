#include "Settings.h"

bool Settings::LoadSettings()
{
	constexpr auto path = L"Data/SKSE/Plugins/po3_SimpleActivateSKSE.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	const auto get_value = [&]<class T>(T& a_value, const char* a_section, const char* a_key, const char* a_comment)
	{
		if constexpr (std::is_same_v<bool, T>) {
			a_value = ini.GetBoolValue(a_section, a_key, a_value);
			ini.SetBoolValue(a_section, a_key, a_value, a_comment);
		} else {
			a_value = ini.GetValue(a_section, a_key, a_value.c_str());
			ini.SetValue(a_section, a_key, a_value.c_str(), a_comment);
		}
	};

	ini.Delete("NPCs", nullptr, true);
	ini.Delete("Doors", nullptr, true);
	ini.Delete("Furniture", nullptr, true);
	ini.Delete("Flora", nullptr, true);
	ini.Delete("Items", nullptr, true);

	get_value(npc.hideAll, "Hide All Text", "NPCs", ";Hide all names and activation prompts");
	get_value(doors.hideAll, "Hide All Text", "Doors", ";");
	get_value(furniture.hideAll, "Hide All Text", "Furniture", ";");
	get_value(flora.hideAll, "Hide All Text", "Flora", ";");
	get_value(items.hideAll, "Hide All Text", "Items", ";");

    get_value(npc.hideButton, "Hide Button", "NPCs", ";Hide activate button, eg. [E]");
	get_value(doors.hideButton, "Hide Button", "Doors", ";");
	get_value(furniture.hideButton, "Hide Button", "Furniture", ";");
	get_value(flora.hideButton, "Hide Button", "Flora", ";");
	get_value(items.hideButton, "Hide Button", "Items", ";");

    get_value(npc.hideText, "Hide Text", "NPCs", ";Hide activate text, eg. Talk, Pickpocket");
	get_value(doors.hideText, "Hide Text", "Doors", ";eg. Open");
	get_value(furniture.hideText, "Hide Text", "Furniture", ";eg. Sleep");
	get_value(flora.hideText, "Hide Text", "Flora", ";eg. Harvest");
	get_value(items.hideText, "Hide Text", "Items", ";eg. Take, Steal");

	get_value(steal.useColoredName, "Steal/Pickpocket", "Show Indicator Using Name", ";Item/NPC names turn red (or custom color defined below).");
	get_value(steal.nameColor, "Steal/Pickpocket", "Custom Indicator Color", ";Color, in hex (default: red)");

	get_value(owned.useColoredName, "Owned", "Show Indicator Using Name", ";Owned furniture name turns yellow (or custom color defined below).");
	get_value(owned.nameColor, "Owned", "Custom Indicator Color", ";Color, in hex (default: yellow)");

	get_value(locked.hideTag, "Locked", "Hide Locked Tag", ";Hide locked status (eg. Apprentice, Adept, Master)");
	get_value(locked.tag, "Locked", "Custom Locked Tag", ";Set custom tag for all locked objects. Leave entry blank if you don't want to set it\n;No effect if Hide Lock Tag is true.");
	get_value(locked.color.useColoredName, "Locked", "Show Indicator Using Name", ";Locked object names turn yellow (or custom color defined below).");
	get_value(locked.color.nameColor, "Locked", "Custom Indicator Color", ";Color, in hex (default: yellow)");

	get_value(empty.hideTag, "Empty", "Hide Empty Tag", ";Hide empty container state");
	get_value(empty.tag, "Empty", "Custom Empty Tag", ";Set custom tag for empty objects (eg. [Empty]). Leave entry blank if you don't want to set it\n;No effect if Hide Empty Tag is true.");
	get_value(empty.color.useColoredName, "Empty", "Show Indicator Using Name", ";Empty container names turn grey (or custom color defined below).");
	get_value(empty.color.nameColor, "Empty", "Custom Indicator Color", ";Color, in hex (default: grey)");

	(void)ini.SaveFile(path);

	return true;
}

std::optional<Settings::Text> Settings::GetText(const RE::TESObjectREFRPtr& a_object)
{
	if (const auto base = a_object ? a_object->GetBaseObject() : nullptr) {
		switch (base->GetFormType()) {
		case RE::FormType::NPC:
			return npc;
		case RE::FormType::Door:
			return doors;
		case RE::FormType::Furniture:
			return furniture;
		case RE::FormType::Flora:
		case RE::FormType::Tree:
			return flora;
		default:
			return items;
		}
	}
	return std::nullopt;
}

std::optional<Settings::Color> Settings::GetColor(const RE::TESObjectREFRPtr& a_object, const std::string& a_text)
{
	if (detail::is_owned(a_object)) {
		return owned;
	}
	if (a_text.find("#FF0000") != std::string::npos) {
		return steal;
	}
	if (detail::is_empty(a_object)) {
		return empty.color;
	}
	if (detail::is_locked(a_object)) {
		return locked.color;
	}
	return std::nullopt;
};

std::optional<Settings::Tag> Settings::GetTag(const RE::TESObjectREFRPtr& a_object)
{
	if (detail::is_empty(a_object)) {
		return empty;
	}
	if (detail::is_locked(a_object)) {
		return locked;
	}
	return std::nullopt;
};
