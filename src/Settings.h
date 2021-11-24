#pragma once

class Settings
{
public:
	struct Text
	{
		bool hideAll{ false };
		bool hideButton{ true };
		bool hideText{ true };
	};

	struct Color
	{
		bool useColoredName;
		std::string nameColor;

		Color(const std::string& a_color) :
			useColoredName(true),
			nameColor(a_color)
		{}
	};

	struct Tag
	{
		bool hideTag;
		std::string tag;
		Color color;

		Tag(const std::string& a_tag, const std::string& a_color) :
			hideTag(false),
			tag(a_tag),
			color(a_color)
		{}
	};

	[[nodiscard]] static Settings* GetSingleton();
	[[nodiscard]] bool LoadSettings();

	std::optional<Text> GetText(const RE::TESObjectREFRPtr& a_object);
	std::optional<Color> GetColor(const RE::TESObjectREFRPtr& a_object, const std::string& a_text);
	std::optional<Tag> GetTag(const RE::TESObjectREFRPtr& a_object);

private:
	Text npc;
	Text doors;
	Text furniture;
	Text items;
	Text flora;

	Color steal{ "#FF0000" };
	Color owned{ "#FFFF00" };

	Tag locked{
		"<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>Locked",
		"#FFFF00"
	};
	Tag empty{
		"",
		"#808080"
	};
};
