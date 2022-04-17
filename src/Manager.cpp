#include "Settings.h"

namespace UI
{
	enum : std::uint32_t
	{
		kPrompt,
		kName,
		kTag
	};

	struct detail
	{
		static bool has_value(const RE::TESObjectREFRPtr& a_object)
		{
            const auto base = a_object ? a_object->GetBaseObject() : nullptr;
			return base && base->GetGoldValue() != -1;
		}

        static std::string get_owned_tag()
		{
            const auto gmst = RE::GameSettingCollection::GetSingleton();
            if (const auto ownedTag = gmst ? gmst->GetSetting("sOwned") : nullptr) {
				std::string tag{ " (" };
				tag.append(ownedTag->GetString()).append(")");
				return tag;
			}
			return {};
		}
    };

	struct UpdatePlayerCrosshairText
	{
		static void thunk(RE::UIMessageQueue* a_this, const RE::BSFixedString& a_menuName, RE::UI_MESSAGE_TYPE a_type, RE::IUIMessageData* a_data)
		{
			if (const auto data = a_data ?
                                      static_cast<RE::HUDData*>(a_data) :
                                      nullptr;
				data) {
                const auto settings = Settings::GetSingleton();

                if (const auto textSettings = settings->GetText(data->crossHairRef.get())) {
					if (textSettings->hideAll) {
						data->type = RE::HUDData::Type::kUnk0;
					} else {
						if (textSettings->hideButton) {
							data->type = RE::HUDData::Type::kActivateNoLabel;
						}
						const std::string origText = data->text.c_str();
						std::string text = origText;

						const auto crossHairRef = data->crossHairRef.get();

                        if (const auto colorSettings = settings->GetColor(crossHairRef, text); colorSettings->useColoredName) {
							auto splitText = string::split(text, "\n");
							if (const auto splitSize = splitText.size(); splitSize > 1) {
                                if (!colorSettings->nameColor.empty()) {
									string::replace_first_instance(splitText[kName], detail::get_owned_tag(), "");

									splitText[kName] = fontBegin + colorSettings->nameColor + fontBeginEnd + splitText[kName] + fontEnd;
									text = string::join(splitText, "\n");
								}
							}
						}

						if (textSettings->hideText) {
                            const bool hasValue = detail::has_value(crossHairRef);
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

                        if (const auto tag = settings->GetTag(crossHairRef)) {
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
		static inline REL::Relocation<decltype(&thunk)> func;

		static inline constexpr const char* fontBegin{ "<font color='" };
		static inline constexpr const char* fontBeginEnd{ "'>" };
		static inline constexpr const char* fontEnd{ "</font>" };
	};

	void Install()
	{
		SKSE::AllocTrampoline(14);

	    REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(39535, 40621), OFFSET(0x289,0x280) };
		stl::write_thunk_call<UpdatePlayerCrosshairText>(target.address());
	}
}
