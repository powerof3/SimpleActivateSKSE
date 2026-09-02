#include "Settings.h"

bool Settings::LoadSettings()
{
	UpdateSettings(path.data());
	
	const auto store = REX::FIniSettingStore::GetSingleton();
	store->Init(path.data(), "");

	store->Load();
	store->Save();

	DetectUseOrTake();

	return true;
}

void Settings::UpdateSettings(const char* a_path)
{
	CSimpleIniA ini;
	ini.SetUnicode();

	if (ini.LoadFile(a_path) < SI_OK) {
		return;
	}

	if (ini.GetValue("Owned", "bShowIndicatorUsingName")) {
		REX::INFO("No settings to migrate...");
		return;
	}

	for (auto& [section, oldKey, newKey] : INI::GetSettingsToUpdate()) {
		CSimpleIniA::TNamesDepend values;
		if (ini.GetAllValues(section.data(), oldKey.data(), values) && !values.empty()) {
			const auto& entry = values.front();
			ini.SetValue(section.data(), newKey.data(), entry.pItem, entry.pComment);
			ini.Delete(section.data(), oldKey.data(), true);
			REX::INFO("Migrated [{}] {} -> {}", section, oldKey, newKey);
		}
	}

	(void)ini.SaveFile(a_path);
}

void Settings::Register()
{
	if (const auto inputMgr = RE::BSInputDeviceManager::GetSingleton()) {
		inputMgr->AddEventSink<RE::InputEvent*>(this);
		REX::INFO("Registered for modifier key event");
	}
}

void Settings::OnDataLoaded()
{
	dakGlobal = RE::TESForm::LookupByEditorID<RE::TESGlobal>("DynamicActivationKey");
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

RE::BSEventNotifyControl Settings::ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*)
{
	if (!a_evn) {
		return RE::BSEventNotifyControl::kContinue;
	}

	const auto settings = Settings::GetSingleton();

	for (auto event = *a_evn; event; event = event->next) {
		const auto buttonEvent = event->AsButtonEvent();
		if (!buttonEvent) {
			continue;
		}

		const auto device = event->GetDevice();
		auto       key = buttonEvent->GetIDCode();

		switch (device) {
		case RE::INPUT_DEVICE::kMouse:
			key += SKSE::InputMap::kMacro_MouseButtonOffset;
			break;
		case RE::INPUT_DEVICE::kGamepad:
			key = SKSE::InputMap::GamepadMaskToKeycode(key);
			break;
		default:
			break;
		}

		const bool isKeyboard = key == (std::uint32_t)settings->modifierKey;
		const bool isGamePad = key == (std::uint32_t)settings->modifierKeyGamePad;

		if (!isKeyboard && !isGamePad) {
			continue;
		}

		const bool held = buttonEvent->IsPressed();
		auto&      state = isGamePad ? settings->modifierHeldGamePad : settings->modifierHeld;

		if (state == held) {
			continue;
		}
		state = held;

		if (const auto pickData = RE::CrosshairPickData::GetSingleton()) {
			const auto& ref = pickData->target.get();
			const auto  baseObj = ref ? ref->GetBaseObject() : nullptr;
			if (baseObj && HasActionLabel(baseObj->GetFormType())) {
				RE::PlayerCharacter::GetSingleton()->UpdateCrosshairs();
			}
		}
	}

	return RE::BSEventNotifyControl::kContinue;
}

bool Settings::HasActionLabel(RE::FormType a_formType) const
{
	return actionLabelTypes.contains(a_formType);
}

bool Settings::ShowActionLabel(RE::FormType a_formType) const
{
	switch (static_cast<ACTION_LABEL>(showActionLabels.GetValue())) {
	case ACTION_LABEL::kAlways:
		return HasActionLabel(a_formType);
	case ACTION_LABEL::kOnModifierPress:
		return IsModifierHeld() && HasActionLabel(a_formType);
	default:
		return false;
	}
}

bool Settings::IsModifierHeld() const
{
	return modifierHeld || modifierHeldGamePad;
}

bool Settings::IsDAKModifierHeld() const
{
	return dakGlobal && dakGlobal->value == 1.0f;
}

void Settings::DetectUseOrTake()
{
	const auto read_key = [](const CSimpleIniA& a_ini, const char* a_key) {
		return static_cast<std::int32_t>(a_ini.GetLongValue("Settings", a_key, -1));
	};

	if (REX::W32::GetModuleHandleA("po3_ReadOrTakeBooks.dll")) {
		actionLabelTypes.insert(RE::FormType::Book);

		CSimpleIniA ini;
		ini.SetUnicode();
		if (ini.LoadFile(R"(Data\SKSE\Plugins\po3_ReadOrTakeBooks.ini)") >= 0) {
			modifierKey = read_key(ini, "iAlternateActionHotkey");
			modifierKeyGamePad = read_key(ini, "iAlternateActionHotkeyGamePad");
		}
	}

	if (REX::W32::GetModuleHandleA("po3_UseOrTake.dll")) {
		CSimpleIniA ini;
		ini.SetUnicode();

		if (ini.LoadFile(R"(Data\SKSE\Plugins\po3_UseOrTake.ini)") < 0) {
			return;
		}

		constexpr auto read_bool = [](const CSimpleIniA& a_ini, const char* a_section) {
			if (const auto value = a_ini.GetValue(a_section, "bEnabled")) {
				return REX::STR::IEQUALS(value, "true"sv) || REX::STR::IEQUALS(value, "1"sv);
			}
			return false;
		};

		constexpr std::array entries{
			std::make_pair("Armors", RE::FormType::Armor),
			std::make_pair("Weapons", RE::FormType::Weapon),
			std::make_pair("Potions", RE::FormType::AlchemyItem),
			std::make_pair("Ingredients", RE::FormType::Ingredient),
			std::make_pair("Scrolls", RE::FormType::Scroll),
			std::make_pair("Torches", RE::FormType::Light),
			std::make_pair("Ammo", RE::FormType::Ammo),
		};

		for (const auto& [section, formType] : entries) {
			if (read_bool(ini, section)) {
				actionLabelTypes.insert(formType);
			}
		}

		if (modifierKey == -1) {
			modifierKey = read_key(ini, "iAlternateActionKey");
		}
		if (modifierKeyGamePad == -1) {
			modifierKeyGamePad = read_key(ini, "iAlternateActionKeyGamePad");
		}
	}
}
