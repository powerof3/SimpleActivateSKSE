#include "Settings.h"

Settings* Settings::GetSingleton()
{
	static Settings singleton;
	return std::addressof(singleton);
}

bool Settings::LoadSettings()
{
	constexpr auto path = L"Data/SKSE/Plugins/po3_SimpleActivateSKSE.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	const auto get_bool_value = [&](bool& a_value, const char* a_section, const char* a_key, const char* a_comment) {
		a_value = ini.GetBoolValue(a_section, a_key, a_value);
		ini.SetBoolValue(a_section, a_key, a_value, a_comment);
	};

	const auto get_value = [&](std::string& a_value, const char* a_section, const char* a_key, const char* a_comment) {
		a_value = ini.GetValue(a_section, a_key, a_value.c_str());
		ini.SetValue(a_section, a_key, a_value.c_str(), a_comment);
	};

	get_bool_value(npc.hideAll, "NPCs", "Hide All Text", ";Hide all NPC names and activation prompts");
	get_bool_value(npc.hideButton, "NPCs", "Hide Button", ";Hide activate button, eg. [E]");
	get_bool_value(npc.hideText, "NPCs", "Hide Text", ";Hide activate text, eg. Talk, Pickpocket");

	get_bool_value(doors.hideAll, "Doors", "Hide All Text", ";Hide all door names and activation prompts");
	get_bool_value(doors.hideButton, "Doors", "Hide Button", ";Hide activate button, eg. [E]");
	get_bool_value(doors.hideText, "Doors", "Hide Text", ";Hide activate text, eg. Open");

	get_bool_value(furniture.hideAll, "Furniture", "Hide All Text", ";Hide all furniture names and activation prompts");
	get_bool_value(furniture.hideButton, "Furniture", "Hide Button", ";Hide activate button, eg. [E]");
	get_bool_value(furniture.hideText, "Furniture", "Hide Text", ";Hide activate text, eg. Sleep");

	get_bool_value(flora.hideAll, "Flora", "Hide All Text", ";Hide all harvestable names and activation prompts");
	get_bool_value(flora.hideButton, "Flora", "Hide Button", ";Hide activate button, eg. [E]");
	get_bool_value(flora.hideText, "Flora", "Hide Text", ";Hide activate text, eg. Harvest");
	
	get_bool_value(items.hideAll, "Items", "Hide All Text", ";Hide all item names and activation prompts");
	get_bool_value(items.hideButton, "Items", "Hide Button", ";Hide activate button, eg. [E]");
	get_bool_value(items.hideText, "Items", "Hide Text", ";Hide activate text, eg. Take, Steal");

	get_bool_value(steal.useColoredName, "Steal/Pickpocket", "Show Indicator Using Name", ";Item/NPC names turn red (or custom color defined below).");
	get_value(steal.nameColor, "Steal/Pickpocket", "Custom Indicator Color", ";Color, in hex (default: red)");

	get_bool_value(owned.useColoredName, "Owned", "Show Indicator Using Name", ";Owned furniture name turns yellow (or custom color defined below).");
	get_value(owned.nameColor, "Owned", "Custom Indicator Color", ";Color, in hex (default: yellow)");

	get_bool_value(locked.hideTag, "Locked", "Hide Locked Tag", ";Hide locked status (eg. Apprentice, Adept, Master)");
	get_value(locked.tag, "Locked", "Custom Locked Tag", ";Set custom tag for all locked objects. Leave entry blank if you don't want to set it\n;No effect if Hide Lock Tag is true.");
	get_bool_value(locked.color.useColoredName, "Locked", "Show Indicator Using Name", ";Locked object names turn yellow (or custom color defined below).");
	get_value(locked.color.nameColor, "Locked", "Custom Indicator Color", ";Color, in hex (default: yellow)");

	get_bool_value(empty.hideTag, "Empty", "Hide Empty Tag", ";Hide empty container state");
	get_value(empty.tag, "Empty", "Custom Empty Tag", ";Set custom tag for empty objects (eg. [Empty]). Leave entry blank if you don't want to set it\n;No effect if Hide Empty Tag is true.");
	get_bool_value(empty.color.useColoredName, "Empty", "Show Indicator Using Name", ";Empty container names turn grey (or custom color defined below).");
	get_value(empty.color.nameColor, "Empty", "Custom Indicator Color", ";Color, in hex (default: grey)");

	ini.SaveFile(path);

	return true;
}

struct detail
{
	static bool is_empty(const RE::TESObjectREFRPtr& a_object)
	{
		return a_object && is_empty_impl(a_object.get(), false, false) == 0;
	}

	static bool is_owned(const RE::TESObjectREFRPtr& a_object)
	{
		auto base = a_object ? a_object->GetBaseObject() : nullptr;
		if (base && base->Is(RE::FormType::Furniture)) {
			return a_object->GetOwner() && a_object->IsOffLimits();
		}
		return false;
	}

	static bool is_locked(const RE::TESObjectREFRPtr& a_object)
	{
		return a_object && a_object->IsLocked();
	};

private:
	static std::int32_t is_empty_impl(const RE::TESObjectREFR* a_object, bool a_useDataHandlerInventory, bool a_unk03)
	{
		using func_t = decltype(&is_empty_impl);
		REL::Relocation<func_t> func{ REL::ID(19274) };
		return func(a_object, a_useDataHandlerInventory, a_unk03);
	};
};

std::optional<Settings::Text> Settings::GetText(const RE::TESObjectREFRPtr& a_object)
{
	auto base = a_object ? a_object->GetBaseObject() : nullptr;
	if (base) {
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
