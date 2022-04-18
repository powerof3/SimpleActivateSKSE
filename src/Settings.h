#pragma once

class Settings
{
public:
    struct Text
	{
		std::string type;
		bool hideAll;
		bool hideButton;
		bool hideText;

		explicit Text(std::string a_type) :
			type(std::move(a_type)),
			hideAll(false),
			hideButton(true),
			hideText(true)
		{}
	};

	struct Color
	{
		bool useColoredName;
		std::string nameColor;

		explicit Color(std::string a_color) :
			useColoredName(true),
			nameColor(std::move(a_color))
		{}
	};

	struct Tag
	{
		bool hideTag;
		std::string tag;
		Color color;

		explicit Tag(std::string a_tag, const std::string& a_color) :
			hideTag(false),
			tag(std::move(a_tag)),
			color(a_color)
		{}
	};

	[[nodiscard]] static Settings* GetSingleton()
	{
		static Settings singleton;
		return std::addressof(singleton);
	}

	[[nodiscard]] bool LoadSettings();

	const Text* GetText(RE::FormType a_formType) const;
	const Text* GetText(const RE::TESObjectREFRPtr& a_object) const;

    const Color* GetColor(const RE::TESObjectREFRPtr& a_object, std::string_view a_text) const;
	const Tag* GetTag(const RE::TESObjectREFRPtr& a_object) const;

private:
	struct detail
	{
		static bool is_empty(const RE::TESObjectREFRPtr& a_object)
		{
			if (get_inventory_count(a_object.get()) == 0) {
				return a_object->IsNot(RE::FormType::ActorCharacter) || a_object->IsDead();
			}
			return false;
		}
		static bool is_owned(const RE::TESObjectREFRPtr& a_object)
		{
			const auto base = a_object->GetBaseObject();
			return base && base->Is(RE::FormType::Furniture) && a_object->GetOwner() && a_object->IsOffLimits();
		}

	private:
		static std::int32_t get_inventory_count(RE::TESObjectREFR* a_object, bool a_useDataHandlerInventory = false, bool a_unk03 = false)
		{
			using func_t = decltype(&get_inventory_count);
			REL::Relocation<func_t> func{ RELOCATION_ID(19274, 19700) };
			return func(a_object, a_useDataHandlerInventory, a_unk03);
		}
	};

	Text activators{ "Activators" };
    Text containers{ "Containers" };
	Text doors{ "Doors" };
	Text furniture{ "Furniture" };
	Text flora{ "Flora" };
	Text items{ "Items" };
	Text npc{ "NPCs" };
	Text projectiles{ "Projectiles" };

	Color steal{ "#FF0000" };
	Color owned{ "#FFFF00" };

	Tag locked{
		"<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>Locked",
		"#FFFF00"
	};
	Tag empty{
		{},
		"#808080"
	};
};
