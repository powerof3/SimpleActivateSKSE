#include "Settings.h"

namespace UI
{
	namespace gmst
	{
		const char* get_string(const char* a_setting)
		{
			return RE::GameSettingCollection::GetSingleton()->GetSetting(a_setting)->GetString();
		}
	}

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
				static std::string tag = std::string(" (").append(gmst::get_string("sOwned")).append(")");
			    return tag;
			}
		};

		struct SendHUDMessage
		{
			static void thunk(RE::UIMessageQueue* a_this, const RE::BSFixedString& a_menuName, RE::UI_MESSAGE_TYPE a_type, RE::IUIMessageData* a_data)
			{
				const auto data = a_data ? static_cast<RE::HUDData*>(a_data) : nullptr;
				const auto crossHairRef = data ? data->crossHairRef.get() : RE::TESObjectREFRPtr();

				if (data && crossHairRef) {
					const auto settings = Settings::GetSingleton();

					if (const auto textSettings = settings->GetText(crossHairRef); textSettings) {
						if (textSettings->hideAll) {
							data->type = RE::HUDData::Type::kUnk0;
						} else {
							if (textSettings->hideButton) {
								data->type = RE::HUDData::Type::kActivateNoLabel;
							}
							const std::string origText = data->text.c_str();
							std::string text = data->text.c_str();

							if (const auto colorSettings = settings->GetColor(crossHairRef, origText); colorSettings && colorSettings->useColoredName) {
								if (auto splitText = string::split(text, "\n"); splitText.size() > 1 && !colorSettings->nameColor.empty()) {
									string::replace_first_instance(splitText[kName], detail::get_owned_tag(), "");

									std::string coloredName{ colored_font };
									string::replace_first_instance(coloredName, "rgba", colorSettings->nameColor);
									string::replace_first_instance(coloredName, "text", splitText[kName]);
									splitText[kName] = coloredName;

									text = string::join(splitText, "\n");
								}
							}

							if (textSettings->hideText) {
								const bool hasValue = detail::has_gold_value(crossHairRef);
								if (auto splitText = string::split(text, "\n"); splitText.size() > 2) {
									if (hasValue && textSettings->hideButton) {
										splitText[kPrompt] = "\n";
									} else {
										splitText.erase(splitText.begin());
									}
									text = string::join(splitText, "\n");
								} else if (splitText.size() > 1) {
									text = hasValue && textSettings->hideButton ? "\n" + splitText[kName] :
									                                              splitText[kName];
								}
							}

							if (const auto tag = settings->GetTag(crossHairRef); tag) {
								if (string::split(origText, "\n").size() > 2) {
									auto splitText = string::split(text, "\n");
									if (tag->hideTag) {
										splitText.pop_back();
										text = string::join(splitText, "\n");
									} else if (!tag->tag.empty()) {
										splitText.back() = tag->tag;
										text = string::join(splitText, "\n");
									}
								}
							}

							if (text != origText) {
								data->text = text;
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
				static std::string tag = std::string(gmst::get_string("sTo")).append(" ");
				return tag;
			}
		};

		struct SendHUDMessage
		{
			static void thunk(RE::UIMessageQueue* a_this, const RE::BSFixedString& a_menuName, RE::UI_MESSAGE_TYPE a_type, RE::IUIMessageData* a_data)
			{
				if (const auto data = a_data ? static_cast<RE::HUDData*>(a_data) : nullptr) {
					if (const auto textSettings = Settings::GetSingleton()->GetText(RE::FormType::Door); textSettings) {
						if (textSettings->hideAll) {
							data->type = RE::HUDData::Type::kUnk0;
						} else {
							const std::string origText = data->text.c_str();
							std::string text = data->text.c_str();

							if (textSettings->hideText) {
								string::replace_first_instance(text, detail::get_To_tag(), "");
							}

							if (text != origText) {
								data->text = text;
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
