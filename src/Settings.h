#pragma once

#include <SimpleIni.h>
#undef ERROR

namespace INI
{
	struct SettingsToUpdate
	{
		std::string_view section;
		std::string_view oldKey;
		std::string_view newKey;
	};

	inline std::vector<SettingsToUpdate>& GetSettingsToUpdate()
	{
		static std::vector<SettingsToUpdate> settings;
		return settings;
	}

	template <class T, class Base = REX::TIniSetting<T>>
	class TSetting : public Base
	{
	public:
		TSetting(std::string_view a_section, std::string_view a_oldKey, std::string_view a_newKey, std::string_view a_comment, T a_default) :
			Base(a_section, a_newKey, std::move(a_default)),
			m_section(a_section),
			m_key(a_newKey),
			m_comment(a_comment)
		{
			if (!a_oldKey.empty() && a_oldKey != a_newKey) {
				GetSettingsToUpdate().emplace_back(a_section, a_oldKey, a_newKey);
			}
		}

		TSetting(std::string_view a_section, std::string_view a_key, std::string_view a_comment, T a_default) :
			TSetting(a_section, {}, a_key, a_comment, std::move(a_default))
		{}

		void Save(void* a_data) override
		{
			Base::Save(a_data);

			auto& ini = *static_cast<CSimpleIniA*>(a_data);

			CSimpleIniA::TNamesDepend values;
			if (!ini.GetAllValues(m_section.data(), m_key.data(), values) || values.empty()) {
				return;
			}

			const auto&       entry = values.front();
			const std::string value{ entry.pItem };

			ini.Delete(m_section.data(), m_key.data());
			ini.SetValue(m_section.data(), m_key.data(), value.c_str(), m_comment.data());
		}

	private:
		std::string_view m_section;
		std::string_view m_key;
		std::string_view m_comment;
	};
}

class Settings :
	public REX::TSingleton<Settings>,
	public RE::BSTEventSink<RE::InputEvent*>

{
public:
	enum class ACTION_LABEL
	{
		kNever = 0,
		kAlways = 1,
		kOnModifierPress = 2
	};

	struct Text
	{
		Text(std::string_view a_type, std::string_view a_key, std::string_view a_allComment = {}, std::string_view a_buttonComment = {}, std::string_view a_textComment = {}) :
			type(a_type),
			hideAll("Hide All Text", a_type, a_key, a_allComment, false),
			hideButton("Hide Button", a_type, a_key, a_buttonComment, true),
			hideText("Hide Text", a_type, a_key, a_textComment, true)
		{}

		// members
		std::string_view type;

		INI::TSetting<bool> hideAll;
		INI::TSetting<bool> hideButton;
		INI::TSetting<bool> hideText;
	};

	struct Color
	{
		Color(std::string_view a_section, std::string_view a_useComment, std::string_view a_colorComment, std::string a_color) :
			useColoredName(a_section, "Show Indicator Using Name"sv, "bShowIndicatorUsingName"sv, a_useComment, true),
			nameColor(a_section, "Custom Indicator Color"sv, "sCustomIndicatorColor"sv, a_colorComment, std::move(a_color))
		{}

		const std::string& GetNameColor() const { return static_cast<const std::string&>(nameColor); }

		// members
		INI::TSetting<bool>        useColoredName;
		INI::TSetting<std::string> nameColor;
	};

	struct Tag
	{
		Tag(std::string_view a_section,
			std::string_view a_hideKey, std::string_view a_newHideKey, std::string_view a_hideComment,
			std::string_view a_tagKey, std::string_view a_newTagKey, std::string_view a_tagComment,
			std::string_view a_useComment, std::string_view a_colorComment,
			std::string a_tag, std::string a_color) :
			hideTag(a_section, a_hideKey, a_newHideKey, a_hideComment, true),
			tag(a_section, a_tagKey, a_newTagKey, a_tagComment, std::move(a_tag)),
			color(a_section, a_useComment, a_colorComment, std::move(a_color))
		{}

		const std::string& GetTag() const { return static_cast<const std::string&>(tag); }

		// members
		INI::TSetting<bool>        hideTag;
		INI::TSetting<std::string> tag;
		Color                      color;
	};

	[[nodiscard]] bool LoadSettings();

	void Register();

	void OnDataLoaded();

	const Text*  GetText(RE::FormType a_formType) const;
	const Text*  GetText(const RE::TESObjectREFRPtr& a_object) const;
	const Color* GetColor(const RE::TESObjectREFRPtr& a_object, std::string_view a_text) const;
	const Tag*   GetTag(const RE::TESObjectREFRPtr& a_object) const;

	[[nodiscard]] bool HasActionLabel(RE::FormType a_formType) const;
	[[nodiscard]] bool ShowActionLabel(RE::FormType a_formType) const;
	[[nodiscard]] bool IsModifierHeld() const;
	[[nodiscard]] bool IsDAKModifierHeld() const;

private:
	struct detail
	{
		static bool is_empty(const RE::TESObjectREFRPtr& a_object)
		{
			std::int32_t inventoryCount = -1;
			if (auto ashPile = a_object->extraList.GetAshPileRef().get()) {
				inventoryCount = ashPile->GetInventoryItemCount();
			} else {
				inventoryCount = a_object->GetInventoryItemCount();
			}
			return inventoryCount == 0 ? a_object->IsNot(RE::FormType::ActorCharacter) || a_object->IsDead() : false;
		}
		static bool is_owned(const RE::TESObjectREFRPtr& a_object)
		{
			const auto base = a_object->GetBaseObject();
			return base && base->Is(RE::FormType::Furniture) && a_object->GetOwner() && a_object->IsOffLimits();
		}
	};

	static constexpr auto path = R"(Data\SKSE\Plugins\po3_SimpleActivateSKSE.ini)"sv;

	RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_evn, RE::BSTEventSource<RE::InputEvent*>*) override;

	void UpdateSettings(const char* a_path);
	
	void DetectUseOrTake();

	// members
	Text npc{
		"NPCs"sv, "bNPCs"sv, ";Hide all names and activation prompts"sv, ";Hide activate button, eg. [E]. This setting may not work if you have HUD mods that bypass vanilla functionality"sv, ";Hide activate text, eg. Talk, Pickpocket, Harvest, Sleep"sv
	};
	Text activators{ "Activators"sv, "bActivators"sv };
	Text containers{ "Containers"sv, "bContainers"sv };
	Text doors{ "Doors"sv, "bDoors"sv };
	Text flora{ "Flora"sv, "bFlora"sv };
	Text furniture{ "Furniture"sv, "bFurniture"sv };
	Text items{ "Items"sv, "bItems"sv };
	Text projectiles{ "Projectiles"sv, "bProjectiles"sv };

	Color steal{ "Steal/Pickpocket"sv, ";Item/NPC names turn red (or custom color defined below)."sv, ";Color, in hex (default: red)"sv, "#FF0000" };
	Color owned{ "Owned"sv, ";Owned furniture name turns yellow (or custom color defined below)."sv, ";Color, in hex (default: yellow)"sv, "#FFFF00" };

	Tag locked{
		"Locked"sv,
		"Hide Locked Tag"sv, "bHideLockedTag"sv, ";Hide locked status (eg. Apprentice, Adept, Master)"sv,
		"Custom Locked Tag"sv, "sCustomLockedTag"sv, ";Set custom tag for all locked objects. Leave entry blank if you don't want to set it\n;No effect if Hide Lock Tag is true.",
		";Locked object names turn yellow (or custom color defined below)."sv, ";Color, in hex (default: yellow)"sv,
		"<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>Locked",
		"#FFFF00"
	};

	Tag empty{
		"Empty"sv,
		"Hide Empty Tag"sv, "bHideEmptyTag"sv, ";Hide empty container state"sv,
		"Custom Empty Tag"sv, "sCustomEmptyTag"sv, ";Set custom tag for empty objects (eg. [Empty]). Leave entry blank if you don't want to set it\n;No effect if Hide Empty Tag is true."sv,
		";Empty container names turn grey (or custom color defined below)."sv, ";Color, in hex (default: grey)"sv,
		{},
		"#808080"
	};

	INI::TSetting<std::uint32_t> showActionLabels{ "Hide Text"sv, "iShowCustomText"sv, ";Show custom activate text from mods like Read Or Take or Use Or Take\n;0 - never | 1 - always | 2 - only while the alternate action key is held"sv, std::to_underlying(ACTION_LABEL::kOnModifierPress) };

	RE::TESGlobal* dakGlobal{ nullptr };
	RE::TESGlobal* dakHotkey{ nullptr };

	std::set<RE::FormType> actionLabelTypes{};

	std::int32_t modifierKey{ -1 };
	std::int32_t modifierKeyGamePad{ -1 };

	std::atomic_bool modifierHeld{ false };
	std::atomic_bool modifierHeldGamePad{ false };
};
