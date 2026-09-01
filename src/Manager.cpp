#include "Settings.h"

namespace UI
{
	namespace CrossHair
	{
		enum : std::uint32_t
		{
			kPrompt,
			kName,
			kTag
		};

		struct detail
		{
			static bool has_gold_value(const RE::TESObjectREFRPtr& a_object)
			{
				const auto base = a_object ? a_object->GetBaseObject() : nullptr;
				return base && base->GetGoldValue() > 0;
			}

			static std::string get_owned_tag()
			{
				static std::string tag = std::string(" (").append(*"sOwned"_gs).append(")");
				return tag;
			}
		};

		struct SendHUDMessage
		{
			static void thunk(RE::UIMessageQueue* a_this, const RE::BSFixedString& a_menuName, RE::UI_MESSAGE_TYPE a_type, RE::HUDData* a_data)
			{
				if (const auto crossHairRef = a_data ? a_data->crosshairRef.get() : RE::TESObjectREFRPtr()) {
					const auto settings = Settings::GetSingleton();

					if (const auto textSettings = settings->GetText(crossHairRef)) {
						if (textSettings->hideAll) {
							a_data->type = RE::HUD_MESSAGE_TYPE::kSetCrosshairTargetTextOnly;
							a_data->text.clear();
						} else {
							if (textSettings->hideButton) {
								a_data->type = RE::HUD_MESSAGE_TYPE::kSetCrosshairTargetTextOnly;
							}

							const std::string origText = a_data->text.c_str();
							std::string text = origText;

							if (const auto colorSettings = settings->GetColor(crossHairRef, origText); colorSettings && colorSettings->useColoredName) {
								if (auto splitText = REX::STR::SPLIT(text, "\n"); splitText.size() > 1 && !colorSettings->GetNameColor().empty()) {
									REX::STR::REPLACE_FIRST_INSTANCE(splitText[kName], detail::get_owned_tag(), "");

									std::string coloredName{ colored_font };
									REX::STR::REPLACE_FIRST_INSTANCE(coloredName, "rgba", colorSettings->GetNameColor());
									REX::STR::REPLACE_FIRST_INSTANCE(coloredName, "text", splitText[kName]);
									splitText[kName] = coloredName;

									text = REX::STR::JOIN(splitText, "\n");
								}
							}

							if (textSettings->hideText) {
								const bool hasValue = detail::has_gold_value(crossHairRef);
								if (auto splitText = REX::STR::SPLIT(text, "\n"); splitText.size() > 2) {
									if (hasValue && textSettings->hideButton) {
										splitText[kPrompt] = "\n";
									} else {
										splitText.erase(splitText.begin());
									}
									text = REX::STR::JOIN(splitText, "\n");
								} else if (splitText.size() > 1) {
									text = hasValue && textSettings->hideButton ? "\n" + splitText[kName] :
									                                              splitText[kName];
								}
							}

							if (const auto tag = settings->GetTag(crossHairRef)) {
								if (REX::STR::SPLIT(origText, "\n").size() > 2) {
									auto splitText = REX::STR::SPLIT(text, "\n");
									if (tag->hideTag) {
										splitText.pop_back();
										text = REX::STR::JOIN(splitText, "\n");
									} else if (const auto& tagStr = tag->GetTag(); !tagStr.empty()) {
										splitText.back() = tagStr;
										text = REX::STR::JOIN(splitText, "\n");
									}
								}
							}

							if (text != origText) {
								a_data->text = text;
							}
						}
					}
				}

				func(a_this, a_menuName, a_type, a_data);
			}
			static inline REL::Relocation<decltype(thunk)> func;

			static inline constexpr auto colored_font{ R"(<font color='rgba'>text</font>)"sv };
		};

		void Install()
		{
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(39535, 40621), OFFSET(0x289, 0x280) };
			stl::write_thunk_call<SendHUDMessage>(target.address());
		}
	}

	namespace AutoLoadDoor
	{
		struct detail
		{
			static std::string get_To_tag()
			{
				static std::string tag = std::string(*"sTo"_gs).append(" ");
				return tag;
			}
		};

		struct SendHUDMessage
		{
			static void thunk(RE::UIMessageQueue* a_this, const RE::BSFixedString& a_menuName, RE::UI_MESSAGE_TYPE a_type, RE::HUDData* a_data)
			{
				if (a_data) {
					if (const auto textSettings = Settings::GetSingleton()->GetText(RE::FormType::Door)) {
						if (textSettings->hideAll) {
							a_data->type = RE::HUD_MESSAGE_TYPE::kSetCrosshairTargetTextOnly;
							a_data->text.clear();
						} else {
							const std::string origText = a_data->text.c_str();
							std::string text = a_data->text.c_str();

							if (textSettings->hideText) {
								REX::STR::REPLACE_FIRST_INSTANCE(text, detail::get_To_tag(), "");
							}

							if (text != origText) {
								a_data->text = text;
							}
						}
					}
				}

				func(a_this, a_menuName, a_type, a_data);
			}
			static inline REL::Relocation<decltype(thunk)> func;
		};

		void Install()
		{
			// 0x22B in 1.6.318
			REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(50727, 51622), OFFSET(0xD7, 0x255) };
			stl::write_thunk_call<SendHUDMessage>(target.address());
		}
	}

	void Install()
	{
		CrossHair::Install();
		AutoLoadDoor::Install();
	}
}
