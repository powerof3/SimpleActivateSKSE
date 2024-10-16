#include "Settings.h"

bool Settings::LoadSettings()
{
	constexpr auto path = L"Data/SKSE/Plugins/po3_SimpleActivateSKSE.ini";

	CSimpleIniA ini;
	ini.SetUnicode();

	ini.LoadFile(path);

	//delete and recreate sections IF old section was found
	if (const auto section = ini.GetSection("NPCs"); section && !section->empty()) {
		ini.Delete("NPCs", nullptr, true);
		ini.Delete("Doors", nullptr, true);
		ini.Delete("Furniture", nullptr, true);
		ini.Delete("Flora", nullptr, true);
		ini.Delete("Items", nullptr, true);
		ini.Delete("Steal/Pickpocket", nullptr, true);
		ini.Delete("Owned", nullptr, true);
		ini.Delete("Locked", nullptr, true);
		ini.Delete("Empty", nullptr, true);
	}

	ini::get_value(ini, npc.hideAll, "Hide All Text", npc.type.c_str(), ";Hide all names and activation prompts");
	ini::get_value(ini, activators.hideAll, "Hide All Text", activators.type.c_str(), nullptr);
	ini::get_value(ini, containers.hideAll, "Hide All Text", containers.type.c_str(), nullptr);
	ini::get_value(ini, doors.hideAll, "Hide All Text", doors.type.c_str(), nullptr);
	ini::get_value(ini, flora.hideAll, "Hide All Text", flora.type.c_str(), nullptr);
	ini::get_value(ini, furniture.hideAll, "Hide All Text", furniture.type.c_str(), nullptr);
	ini::get_value(ini, items.hideAll, "Hide All Text", items.type.c_str(), nullptr);
	ini::get_value(ini, projectiles.hideAll, "Hide All Text", projectiles.type.c_str(), nullptr);

	ini::get_value(ini, npc.hideButton, "Hide Button", npc.type.c_str(), ";Hide activate button, eg. [E]. This setting may not work if you have HUD mods that bypass vanilla functionality");
	ini::get_value(ini, activators.hideButton, "Hide Button", activators.type.c_str(), nullptr);
	ini::get_value(ini, containers.hideButton, "Hide Button", containers.type.c_str(), nullptr);
	ini::get_value(ini, doors.hideButton, "Hide Button", doors.type.c_str(), nullptr);
	ini::get_value(ini, flora.hideButton, "Hide Button", flora.type.c_str(), nullptr);
	ini::get_value(ini, furniture.hideButton, "Hide Button", furniture.type.c_str(), nullptr);
	ini::get_value(ini, items.hideButton, "Hide Button", items.type.c_str(), nullptr);
	ini::get_value(ini, projectiles.hideButton, "Hide Button", projectiles.type.c_str(), nullptr);

	ini::get_value(ini, npc.hideText, "Hide Text", npc.type.c_str(), ";Hide activate text, eg. Talk, Pickpocket, Harvest, Sleep");
	ini::get_value(ini, activators.hideText, "Hide Text", activators.type.c_str(), nullptr);
	ini::get_value(ini, containers.hideText, "Hide Text", containers.type.c_str(), nullptr);
	ini::get_value(ini, doors.hideText, "Hide Text", doors.type.c_str(), nullptr);
	ini::get_value(ini, flora.hideText, "Hide Text", flora.type.c_str(), nullptr);
	ini::get_value(ini, furniture.hideText, "Hide Text", furniture.type.c_str(), nullptr);
	ini::get_value(ini, items.hideText, "Hide Text", items.type.c_str(), nullptr);
	ini::get_value(ini, projectiles.hideText, "Hide Text", projectiles.type.c_str(), nullptr);

	ini::get_value(ini, steal.useColoredName, "Steal/Pickpocket", "Show Indicator Using Name", ";Item/NPC names turn red (or custom color defined below).");
	ini::get_value(ini, steal.nameColor, "Steal/Pickpocket", "Custom Indicator Color", ";Color, in hex (default: red)");

	ini::get_value(ini, owned.useColoredName, "Owned", "Show Indicator Using Name", ";Owned furniture name turns yellow (or custom color defined below).");
	ini::get_value(ini, owned.nameColor, "Owned", "Custom Indicator Color", ";Color, in hex (default: yellow)");

	ini::get_value(ini, locked.hideTag, "Locked", "Hide Locked Tag", ";Hide locked status (eg. Apprentice, Adept, Master)");
	ini::get_value(ini, locked.tag, "Locked", "Custom Locked Tag", ";Set custom tag for all locked objects. Leave entry blank if you don't want to set it\n;No effect if Hide Lock Tag is true.");
	ini::get_value(ini, locked.color.useColoredName, "Locked", "Show Indicator Using Name", ";Locked object names turn yellow (or custom color defined below).");
	ini::get_value(ini, locked.color.nameColor, "Locked", "Custom Indicator Color", ";Color, in hex (default: yellow)");

	ini::get_value(ini, empty.hideTag, "Empty", "Hide Empty Tag", ";Hide empty container state");
	ini::get_value(ini, empty.tag, "Empty", "Custom Empty Tag", ";Set custom tag for empty objects (eg. [Empty]). Leave entry blank if you don't want to set it\n;No effect if Hide Empty Tag is true.");
	ini::get_value(ini, empty.color.useColoredName, "Empty", "Show Indicator Using Name", ";Empty container names turn grey (or custom color defined below).");
	ini::get_value(ini, empty.color.nameColor, "Empty", "Custom Indicator Color", ";Color, in hex (default: grey)");

	(void)ini.SaveFile(path);

	return true;
}

const Settings::Text* Settings::GetText(const RE::FormType a_formType) const
{
	switch (a_formType) {
	case RE::FormType::Activator:
		return &activators;
	case RE::FormType::Container:
		return &containers;
	case RE::FormType::NPC:
		return &npc;
	case RE::FormType::Door:
		return &doors;
	case RE::FormType::Furniture:
		return &furniture;
	case RE::FormType::Projectile:
		return &projectiles;
	case RE::FormType::Flora:
	case RE::FormType::Tree:
		return &flora;
	default:
		return &items;
	}
}

const Settings::Text* Settings::GetText(const RE::TESObjectREFRPtr& a_object) const
{
	const auto base = a_object->GetBaseObject();
	return base ? GetText(base->GetFormType()) : nullptr;
}

const Settings::Color* Settings::GetColor(const RE::TESObjectREFRPtr& a_object, std::string_view a_text) const
{
	if (detail::is_owned(a_object)) {
		return &owned;
	}
	if (a_text.contains("#FF0000"sv)) {
		return &steal;
	}
	if (detail::is_empty(a_object)) {
		return &empty.color;
	}
	if (a_object->IsLocked()) {
		return &locked.color;
	}

	return nullptr;
}

const Settings::Tag* Settings::GetTag(const RE::TESObjectREFRPtr& a_object) const
{
	if (detail::is_empty(a_object)) {
		return &empty;
	}
	if (a_object->IsLocked()) {
		return &locked;
	}
	return nullptr;
}
