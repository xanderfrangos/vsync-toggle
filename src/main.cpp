// system includes
#include <iostream>
#include <set>
#include <map>
#include <stdexcept>
#include <vector>

// local includes
#include "nvapiwrapper/nvapidrssession.h"
#include "nvapiwrapper/utils.h"

//--------------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
    try
    {
        const std::set<std::string>    possible_values{"status", "default", "off", "on", "app", "fast", "half", "third", "quarter"};
        typedef std::map<std::string, NvU32> settings_map;
        settings_map available_settings;
        const std::vector<std::string> args(argv, argv + argc);

        available_settings.insert({std::string("default"), VSYNCMODE_DEFAULT});
        available_settings.insert({std::string("off"), VSYNCMODE_FORCEOFF});
        available_settings.insert({std::string("on"), VSYNCMODE_FORCEON});
        available_settings.insert({std::string("app"), VSYNCMODE_PASSIVE});
        available_settings.insert({std::string("fast"), VSYNCMODE_VIRTUAL});
        available_settings.insert({std::string("half"), VSYNCMODE_FLIPINTERVAL2});
        available_settings.insert({std::string("third"), VSYNCMODE_FLIPINTERVAL3});
        available_settings.insert({std::string("quarter"), VSYNCMODE_FLIPINTERVAL4});

        if (args.size() != 2 || !possible_values.contains(args[1]))
        {
            // clang-format off
            std::cout << std::endl;
            std::cout << "  Usage example:" << std::endl;
            std::cout << "    vsynctoggle status    prints the current VSync status" << std::endl;
            std::cout << "    vsynctoggle default   lets the game decide the VSync status" << std::endl;
            std::cout << "    vsynctoggle off       turns VSync off" << std::endl;
            std::cout << "    vsynctoggle on        turns VSync on" << std::endl;
            std::cout << "    vsynctoggle fast      turns VSync on with Fast Sync" << std::endl;
            std::cout << "    vsynctoggle half      turns VSync on at 1/2 rate" << std::endl;
            std::cout << "    vsynctoggle third     turns VSync on at 1/3 rate" << std::endl;
            std::cout << "    vsynctoggle quarter   turns VSync on at 1/4 rate" << std::endl;
            std::cout << std::endl;
            // clang-format on
            return args.size() < 2 ? EXIT_SUCCESS : EXIT_FAILURE;
        }

        NvApiWrapper       nvapi;
        NvApiDrsSession    drs_session;
        NvDRSProfileHandle drs_profile{nullptr};
        NVDRS_SETTING      drs_setting{};

        drs_setting.version = NVDRS_SETTING_VER;

        assertSuccess(nvapi.DRS_LoadSettings(drs_session), "Failed to load session settings!");
        assertSuccess(nvapi.DRS_GetBaseProfile(drs_session, &drs_profile), "Failed to get base profile!");

        // Handle special case of getting settings
        {
            NvU32      retrieved_value{VSYNCMODE_DEFAULT};
            const auto status{nvapi.DRS_GetSetting(drs_session, drs_profile, VSYNCMODE_ID, &drs_setting)};
            if (status != NVAPI_SETTING_NOT_FOUND)
            {
                assertSuccess(status, "Failed to get VSync setting!");
                retrieved_value = drs_setting.u32CurrentValue;
            }

            drs_setting                 = {};
            drs_setting.version         = NVDRS_SETTING_VER;
            drs_setting.settingId       = VSYNCMODE_ID;
            drs_setting.settingType     = NVDRS_DWORD_TYPE;
            drs_setting.settingLocation = NVDRS_CURRENT_PROFILE_LOCATION;
            drs_setting.u32CurrentValue = retrieved_value;
        }

        if (args[1] == "status")
        {
            for(auto &it : available_settings) { 
                if(it.second == drs_setting.u32CurrentValue) { 
                    std::cout << it.first << std::endl;
                } 
            }
            return EXIT_SUCCESS;
        }

        auto found_setting = available_settings.find(args[1]);
        if (found_setting == available_settings.end()) {
            throw std::runtime_error("Unsupported VSync mode specified!");
        }

        auto requested_setting = found_setting->second;
        if (requested_setting == drs_setting.u32CurrentValue)
        {
            return EXIT_SUCCESS;
        }

        drs_setting.u32CurrentValue = requested_setting;

        assertSuccess(nvapi.DRS_SetSetting(drs_session, drs_profile, &drs_setting), "Failed to set VSync setting!");
        assertSuccess(nvapi.DRS_SaveSettings(drs_session), "Failed to save session settings!");
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
