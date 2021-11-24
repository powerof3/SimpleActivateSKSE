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
			auto base = a_object ? a_object->GetBaseObject() : nullptr;
			return base && base->GetGoldValue() != -1;
		};
		
		static std::string get_owned_tag()
		{
			auto gmst = RE::GameSettingCollection::GetSingleton();
			auto ownedTag = gmst ? gmst->GetSetting("sOwned") : nullptr;
			if (ownedTag) {
				std::string tag{ " (" };
				tag.append(ownedTag->GetString());
				tag.append(")");
				return tag;
			}
			return std::string();
		};
	};

	struct UpdatePlayerCrosshairText
	{
		static void thunk(RE::UIMessageQueue* a_this, const RE::BSFixedString& a_menuName, RE::UI_MESSAGE_TYPE a_type, RE::IUIMessageData* a_data)
		{
			if (auto data = a_data ?
                                static_cast<RE::HUDData*>(a_data) :
                                nullptr;
				data) {
				auto settings = Settings::GetSingleton();
				
				auto textSettings = settings->GetText(data->crossHairRef.get());
				if (textSettings) {
					if (textSettings->hideAll) {
						data->type = RE::HUDData::Type::kUnk0;
					} else {
						if (textSettings->hideButton) {
							data->type = RE::HUDData::Type::kActivateNoLabel;
						}
						const std::string origText = data->text.c_str();
						std::string text = origText;

						const auto crossHairRef = data->crossHairRef.get();

						auto colorSettings = settings->GetColor(crossHairRef, text);
						if (colorSettings && colorSettings->useColoredName) {
							auto splitText = string::split(text, "\n");
							if (const auto splitSize = splitText.size(); splitSize > 1) {
								std::string nameColor = colorSettings->nameColor;
								if (!nameColor.empty()) {
									string::replace_first_instance(splitText[kName], detail::get_owned_tag(), "");
									
									splitText[kName] = fontBegin + nameColor + fontBeginEnd + splitText[kName] + fontEnd;
									text = string::join(splitText, "\n");
								}
							}
						}

						if (textSettings->hideText) {
							bool hasValue = detail::has_value(crossHairRef);
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

						auto tag = settings->GetTag(crossHairRef);
						if (tag) {
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
		REL::Relocation<std::uintptr_t> target{ REL::ID(39535), 0x289 };
		stl::write_thunk_call<UpdatePlayerCrosshairText>(target.address());
	}
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "SimpleActivateSKSE";
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	logger::info("loaded plugin");

	SKSE::Init(a_skse);

	if (Settings::GetSingleton()->LoadSettings()) {
		SKSE::AllocTrampoline(14);
		UI::Install();
	}

	return true;
}
