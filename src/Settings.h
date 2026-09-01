#pragma once

class Settings : public REX::TSingleton<Settings>
{
public:
	struct Text
	{
		explicit Text(std::string_view a_type) :
			type(a_type),
			hideAll("Hide All Text", a_type, false),
			hideButton("Hide Button", a_type, true),
			hideText("Hide Text", a_type, true)
		{}

		// members
		std::string_view type;

		REX::TIniSetting<bool> hideAll;
		REX::TIniSetting<bool> hideButton;
		REX::TIniSetting<bool> hideText;
	};

	struct Color
	{
		Color(std::string_view a_section, std::string a_color) :
			useColoredName(a_section, "Show Indicator Using Name", true),
			nameColor(a_section, "Custom Indicator Color", std::move(a_color))
		{}

		const std::string& GetNameColor() const
		{
			return static_cast<const std::string&>(nameColor);
		}

		std::string GetColoredName(std::string_view a_name) const
		{
			if (!useColoredName || a_name.empty()) {
				return std::string{ a_name };
			}
			return std::format("<font color='{}'>{}</font>", GetNameColor(), a_name);
		}

		// members
		REX::TIniSetting<bool>        useColoredName;
		REX::TIniSetting<std::string> nameColor;
	};

	struct Tag
	{
		Tag(std::string_view a_section, std::string_view a_hideKey, std::string_view a_tagKey,
			std::string a_tag, std::string a_color) :
			hideTag(a_section, a_hideKey, true),
			tag(a_section, a_tagKey, std::move(a_tag)),
			color(a_section, std::move(a_color))
		{}

		const std::string& GetTag() const
		{
			return static_cast<const std::string&>(tag);
		}

		// members
		REX::TIniSetting<bool>        hideTag;
		REX::TIniSetting<std::string> tag;
		Color                         color;
	};

	[[nodiscard]] bool LoadSettings();

	const Text*  GetText(RE::FormType a_formType) const;
	const Text*  GetText(const RE::TESObjectREFRPtr& a_object) const;
	const Color* GetColor(const RE::TESObjectREFRPtr& a_object, std::string_view a_text) const;
	const Tag*   GetTag(const RE::TESObjectREFRPtr& a_object) const;

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

	// members
	Text activators{ "Activators"sv };
	Text containers{ "Containers"sv };
	Text doors{ "Doors"sv };
	Text furniture{ "Furniture"sv };
	Text flora{ "Flora"sv };
	Text items{ "Items"sv };
	Text npc{ "NPCs"sv };
	Text projectiles{ "Projectiles"sv };

	Color steal{ "Steal/Pickpocket"sv, "#FF0000" };
	Color owned{ "Owned"sv, "#FFFF00" };

	Tag locked{
		"Locked"sv, "Hide Locked Tag"sv, "Custom Locked Tag"sv,
		"<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>Locked",
		"#FFFF00"
	};
	Tag empty{
		"Empty"sv, "Hide Empty Tag"sv, "Custom Empty Tag"sv,
		{},
		"#808080"
	};
};
