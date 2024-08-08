#include "../helpers/colors.hpp"
#include "../helpers/helpers.hpp"

#include "../defines.hpp"
#include "../de_common/configFile.hpp"
#include "../de_common/localConfigFile.hpp"
#include "sdr_main.hpp"
#include "sdr_facade.hpp"
#include "sdr_driver.hpp"



void de::sdr::CSDRMain::loopScheduler()
{
    while (!m_exit_thread)
    {
        try
        {
            // timer each 10m sec.
            wait_time_nsec (0,10000000);

            m_counter++;

            if (m_counter%1000 ==0)
            {   // each 1000 msec
                
                //CSDRDriver::getInstance().connect();
            }
            if (m_counter%1000 ==0)
            {   // each 10000 msec
                
                // CSDR_Facade::getInstance().API_SDRInfo("");
                CSDR_Facade::getInstance().sendLocationInfo("");
            }
        }
        catch (int error)
        {

        }
    }

    return ;
}


void de::sdr::CSDRMain::initSDRParameters()
{
    de::CConfigFile& cConfigFile = de::CConfigFile::getInstance();
    de::CLocalConfigFile& cLocalConfigFile = de::CLocalConfigFile::getInstance();

    std::string m_driver = cLocalConfigFile.getStringField("driver");
    if (m_driver=="")
    {
        const Json_de& jsonConfig = cConfigFile.GetConfigJSON();

        if (jsonConfig.contains("sdr_config"))
        {
            const Json_de sdr_config = jsonConfig["sdr_config"];
            if (validateField(sdr_config, "driver",Json_de::value_t::string))
            {
                m_driver = sdr_config["driver"].get<std::string>();
                cLocalConfigFile.addStringField("driver",m_driver.c_str());
                cLocalConfigFile.apply();
            }
            else
            {
                std::cout << _ERROR_CONSOLE_BOLD_TEXT_ << "FATAL ERROR:" << _INFO_CONSOLE_TEXT << " Missing field " <<  _TEXT_BOLD_HIGHTLITED_ << "sdr_config.driver" <<  _INFO_CONSOLE_TEXT << " in config.module.json" <<   "is missing in config file." << _NORMAL_CONSOLE_TEXT_ << std::endl;    
                exit(0);
            }
        }
        else
        {
            // FATA Error JSON FILE IS MISSING SDR SETTINGS
            std::cout << _ERROR_CONSOLE_BOLD_TEXT_ << "FATAL ERROR:" << _INFO_CONSOLE_TEXT << " Missing field " <<  _TEXT_BOLD_HIGHTLITED_ << "sdr_config" <<  _INFO_CONSOLE_TEXT << " in " <<  configName << "is missing in config file." << _NORMAL_CONSOLE_TEXT_ << std::endl;    
            exit(0);
        }
    }


    if (!cLocalConfigFile.getDecimalField("lng", m_longitude))
    {
                
        const Json_de& jsonConfig = cConfigFile.GetConfigJSON();

        if (!jsonConfig.contains("sdr_config"))
        {
            m_report_static_location = false;
            std::cout << _INFO_CONSOLE_TEXT << "Warning " << _LOG_CONSOLE_BOLD_TEXT << " No static location is defined for the unit (sdr_config.location)" <<  _INFO_CONSOLE_BOLD_TEXT << " No Location will be reported. " << _NORMAL_CONSOLE_TEXT_ << std::endl;    
            return ;
        }

        const Json_de sdr_config = jsonConfig["sdr_config"];
        if (sdr_config.contains("location"))
        {
            try
            {
                const Json_de unit_location = sdr_config["location"];
                        
                m_longitude = unit_location["lng"].get<double>();
                m_latitude = unit_location["lat"].get<double>();
                                            
                cLocalConfigFile.addDecimalField("lng", m_longitude);
                cLocalConfigFile.addDecimalField("lat", m_latitude);
                cLocalConfigFile.apply();
                m_report_static_location = true;
            }
            catch (...)
            {
                m_report_static_location = false;
                std::cout << _INFO_CONSOLE_TEXT << "Warning " << _LOG_CONSOLE_BOLD_TEXT << " No static location is defined for the unit (sdr_config.location)" <<  _INFO_CONSOLE_BOLD_TEXT << " No Location will be reported. " << _NORMAL_CONSOLE_TEXT_ << std::endl;    
            }
        }
        else
        {
            m_report_static_location = false;
            std::cout << _INFO_CONSOLE_TEXT << "Warning " << _LOG_CONSOLE_BOLD_TEXT << " No static location is defined for the unit (sdr_config.location)" <<  _INFO_CONSOLE_BOLD_TEXT << " No Location will be reported. " << _NORMAL_CONSOLE_TEXT_ << std::endl;    
        }
    }
    else
    {
        cLocalConfigFile.getDecimalField("lat", m_latitude);
        m_report_static_location = true;
    }
    
    
    std::cout << _LOG_CONSOLE_BOLD_TEXT << "SDR Driver: " << _INFO_CONSOLE_TEXT << m_driver << _NORMAL_CONSOLE_TEXT_ << std::endl;
    std::cout << _LOG_CONSOLE_BOLD_TEXT << "Location (lng,lat): " << _INFO_CONSOLE_TEXT << std::to_string(m_longitude) << "," << std::to_string(m_latitude) << _NORMAL_CONSOLE_TEXT_ << std::endl;
    
}

bool de::sdr::CSDRMain::init()
{

    initSDRParameters();


    CSDRDriver::getInstance().setSDRDriverIndex(0);
    CSDRDriver::getInstance().init();

    m_exit_thread = false; 


    m_scheduler_thread = std::thread{[&](){ loopScheduler(); }};

    return true;
}

bool de::sdr::CSDRMain::uninit()
{
    return true;
}