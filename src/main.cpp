class Settings
{
public:
	static Settings* GetSingleton()
	{
		static Settings singleton;
		return std::addressof(singleton);
	}

	bool LoadSettings()
	{
		constexpr auto path = L"Data/SKSE/Plugins/po3_SimpleActivateSKSE.ini";

		CSimpleIniA ini;
		ini.SetUnicode();

		ini.LoadFile(path);

		const auto get_bool_value = [&](bool& a_value, const char* a_section, const char* a_key, const char* a_comment) {
			a_value = ini.GetBoolValue(a_section, a_key, a_value);
			ini.SetBoolValue(a_section, a_key, a_value, a_comment);
		};

		const auto get_value = [&](std::string& a_value, const char* a_section, const char* a_key, const char* a_comment) {
			a_value = ini.GetValue(a_section, a_key, a_value.c_str());
			ini.SetValue(a_section, a_key, a_value.c_str(), a_comment);
		};

		get_bool_value(hideNPCs, "Override", "Hide All NPCs", ";Hide all NPC names and activation prompts (including steal/pickpocket indicator)");
		get_bool_value(hideDoors, "Override", "Hide All Doors", ";Hide all door names and activation prompts.");
		get_bool_value(hideFurniture, "Override", "Hide All Furniture", ";Hide all furniture names and activation prompts (included owned indicator).");
		get_bool_value(hideItems, "Override", "Hide All Items", ";Hide all item names and activation prompts (including steal/pickpocket/item value indicator)");

		get_bool_value(hideButton, "Main", "Hide Button", ";Hide activate button, eg. [E]");
		get_bool_value(hideText, "Main", "Hide Activate Text", ";Hide activate text, eg. Take, Harvest, Steal");

		get_bool_value(steal.useColoredName, "Steal/Pickpocket", "Show Indicator Using Name", ";Item/NPC names turn red (or custom color defined below).");
		get_value(steal.nameColor, "Steal/Pickpocket", "Custom Indicator Color", ";Color, in hex (default: red)");

		get_bool_value(owned.useColoredName, "Owned", "Show Indicator Using Name", ";Owned furniture name turns yellow (or custom color defined below).");
		get_value(owned.nameColor, "Owned", "Custom Indicator Color", ";Color, in hex (default: yellow)");

		get_bool_value(locked.hideTag, "Locked", "Hide Locked Tag", ";Hide locked status (eg. Apprentice, Adept, Master)");
		get_value(locked.tag, "Locked", "Custom Locked Tag", ";Set custom tag for all locked objects. Leave entry blank if you don't want to set it\n;No effect if Hide Lock Tag is true.");
		get_bool_value(locked.useColoredName, "Locked", "Show Indicator Using Name", ";Locked object names turn yellow (or custom color defined below).");
		get_value(locked.nameColor, "Locked", "Custom Indicator Color", ";Color, in hex (default: yellow)");

		get_bool_value(empty.hideTag, "Empty", "Hide Empty Tag", ";Hide empty container state");
		get_value(empty.tag, "Empty", "Custom Empty Tag", ";Set custom tag for empty objects (eg. [Empty]). Leave entry blank if you don't want to set it\n;No effect if Hide Empty Tag is true.");
		get_bool_value(empty.useColoredName, "Empty", "Show Indicator Using Name", ";Empty container names turn grey (or custom color defined below).");
		get_value(empty.nameColor, "Empty", "Custom Indicator Color", ";Color, in hex (default: grey)");

		usesColoredName = steal.useColoredName || owned.useColoredName || locked.useColoredName || empty.useColoredName;
		hidesTag = locked.hideTag || empty.hideTag;
		usesCustomTag = !locked.tag.empty() || !empty.tag.empty();

		ini.SaveFile(path);

		return true;
	}

	bool hideNPCs{ false };
	bool hideDoors{ false };
	bool hideFurniture{ false };
	bool hideItems{ false };

	bool hideButton{ true };
	bool hideText{ true };

	struct
	{
		bool useColoredName{ true };
		std::string nameColor{ "#FF0000" };
	} steal;

	struct
	{
		bool useColoredName{ true };
		std::string nameColor{ "#FFFF00" };
	} owned;

	struct
	{
		bool hideTag{ false };
		std::string tag{ "<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>Locked" };
		bool useColoredName{ false };
		std::string nameColor{ "#FFFF00" };
	} locked;

	struct
	{
		bool hideTag{ false };
		std::string tag{};
		bool useColoredName{ false };
		std::string nameColor{ "#808080" };
	} empty;

	bool usesColoredName{ false };
	bool hidesTag{ false };
	bool usesCustomTag{ false };
};

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
		static bool can_hide(const Settings* a_settings, const RE::TESObjectREFRPtr& a_object)
		{
			auto base = a_object ? a_object->GetBaseObject() : nullptr;
			if (base) {
				switch (base->GetFormType()) {
				case RE::FormType::NPC:
					return a_settings->hideNPCs;
				case RE::FormType::Door:
					return a_settings->hideDoors;
				case RE::FormType::Furniture:
					return a_settings->hideFurniture;
				default:
					return a_settings->hideItems;
				}
			}
			return false;
		}

		static bool has_value(const RE::TESObjectREFRPtr& a_object)
		{
			auto base = a_object ? a_object->GetBaseObject() : nullptr;
			return base && base->GetGoldValue() != -1;
		};

		static bool is_empty(const RE::TESObjectREFRPtr& a_object)
		{
			return a_object && is_empty_impl(a_object.get(), false, false) == 0;
		}

		static bool is_furniture(const RE::TESObjectREFRPtr& a_object)
		{
			auto base = a_object ? a_object->GetBaseObject() : nullptr;
			return base && base->Is(RE::FormType::Furniture);
		}

		static bool is_locked(const RE::TESObjectREFRPtr& a_object)
		{
			return a_object && a_object->IsLocked();
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

	private:
		static std::int32_t is_empty_impl(const RE::TESObjectREFR* a_object, bool a_useDataHandlerInventory, bool a_unk03)
		{
			using func_t = decltype(&is_empty_impl);
			REL::Relocation<func_t> func{ REL::ID(19274) };
			return func(a_object, a_useDataHandlerInventory, a_unk03);
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
				if (detail::can_hide(settings, data->crossHairRef.get())) {
					data->type = RE::HUDData::Type::kUnk0;

				} else {
					if (settings->hideButton) {
						data->type = RE::HUDData::Type::kActivateNoLabel;
					}

					const std::string origText = data->text.c_str();
					std::string text = origText;

					const auto crossHairRef = data->crossHairRef.get();

					if (settings->usesColoredName) {
						auto splitText = string::split(text, "\n");
						if (const auto splitSize = splitText.size(); splitSize > 1) {
							std::string nameColor;
							if (detail::is_furniture(crossHairRef) && settings->owned.useColoredName) {
								if (auto ownedTag = detail::get_owned_tag(); text.find(ownedTag) != std::string::npos) {
									string::replace_first_instance(splitText[kName], ownedTag, "");

									nameColor = settings->owned.nameColor;
								}
							} else if (text.find("#FF0000") != std::string::npos && settings->steal.useColoredName) {
								nameColor = settings->steal.nameColor;
							} else if (detail::is_empty(crossHairRef) && settings->empty.useColoredName) {
								nameColor = settings->empty.nameColor;
							} else if (detail::is_locked(crossHairRef) && settings->locked.useColoredName) {
								nameColor = settings->locked.nameColor;
							}
							if (!nameColor.empty()) {
								splitText[kName] = fontBegin + nameColor + fontBeginEnd + splitText[kName] + fontEnd;
								text = string::join(splitText, "\n");
							}
						}
					}

					if (settings->hideText) {
						bool hasValue = detail::has_value(crossHairRef);
						if (auto splitText = string::split(text, "\n"); splitText.size() > 2) {
							if (hasValue && settings->hideButton) {
								splitText[kPrompt] = "\n";
							} else {
								splitText.erase(splitText.begin());
							}
							text = string::join(splitText, "\n");
						} else if (splitText.size() > 1) {
							text = hasValue && settings->hideButton ? "\n" + splitText[kName] :
                                                                      splitText[kName];
						}
					}

					if (settings->hidesTag || settings->usesCustomTag) {
						if (string::split(origText, "\n").size() > 2) {
							auto splitText = string::split(text, "\n");

							if (detail::is_empty(crossHairRef)) {
								if (settings->empty.hideTag) {
									splitText.pop_back();
									text = string::join(splitText, "\n");
								} else if (!settings->empty.tag.empty()) {
									splitText.back() = settings->empty.tag;
									text = string::join(splitText, "\n");
								}
							} else if (detail::is_locked(crossHairRef)) {
								if (settings->locked.hideTag) {
									splitText.pop_back();
									text = string::join(splitText, "\n");
								} else if (!settings->locked.tag.empty()) {
									splitText.back() = settings->locked.tag;
									text = string::join(splitText, "\n");
								}
							}
						}
					}

					if (text != origText) {
						data->text = text;
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
		REL::Relocation<std::uintptr_t> target{ REL::ID(39535) };

		stl::write_thunk_call<UpdatePlayerCrosshairText>(target.address() + 0x289);
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
