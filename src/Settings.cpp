#include "Settings.h"

bool Settings::LoadSettings()
{
	const auto store = REX::FIniSettingStore::GetSingleton();
	store->Init(path.data(), "");

	store->Load();
	store->Save();

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
