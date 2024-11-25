#include "../defines.hpp"

#include "../helpers/helpers.hpp"

#include "sdr_facade.hpp"
#include "sdr_main.hpp"
#include "sdr_driver.hpp"

using namespace de::sdr;

using namespace de::comm;


void CSDR_Facade::API_SDRInfo(const std::string&target_party_id) const
{
    //de::andruav_servers::CP2P& cP2P = de::andruav_servers::CP2P::getInstance();
    CSDRDriver& cSDRDriver  = CSDRDriver::getInstance();
    
    Json_de jMsg = 
        {
            {"a", SDR_ACTION_SDR_INFO},
            {"c" , cSDRDriver.getStatus()},
            {"fc", cSDRDriver.getFrequencyCenter()},
            {"s" , cSDRDriver.getSampleRate()},
            {"g" , cSDRDriver.getGain()},
            {"m" , cSDRDriver.getDecodeMode()},
            {"i" , cSDRDriver.getSDRDriverIndex()},
            {"r" , cSDRDriver.getBars()},
            {"t" , cSDRDriver.getIntervals()},
            {"l" , cSDRDriver.getTriggerLevel()}
        };

    #ifdef DDEBUG
        std::cout << "XXXXXXXXXXXXXXXXXXXXX" << jMsg.dump() << std::endl;
    #endif
    
    m_module.sendJMSG (target_party_id, jMsg, TYPE_AndruavMessage_SDR_ACTION,  false);
    
}

void CSDR_Facade::API_SendSDRDrivers (const std::string& target_party_id) const
{
    CSDRDriver& cSDRDriver  = CSDRDriver::getInstance();
    cSDRDriver.listDevices();
    std::vector<SoapySDR::Kwargs> device_args = cSDRDriver.get_device_agrs();
    SoapySDR::Kwargs device_arg;
    
    Json_de drivers_array; 
    Json_de driver;

    for(unsigned int k=0;k<device_args.size();++k)
    {
		device_arg = device_args[k];
        int driver_index = 0;
		for (SoapySDR::Kwargs::const_iterator it = device_arg.begin(); it != device_arg.end(); ++it)
        {
            std::cout << it->first << " = " << it->second << std::endl;

            if (it->first == "index")
            {
                std::cout << it->first << " == " << it->second << std::endl;
                driver_index = std::atoi(it->second.c_str());
                driver[it->first] = driver_index;
            }
            else
            {
                driver[it->first] = it->second;
            }
		}

        drivers_array[driver_index] = driver;
        
    }

    
    Json_de jMsg = 
        {
            {"a" , SDR_ACTION_LIST_SDR_DEVICES},
            {"dr", drivers_array},
        };

    m_module.sendJMSG (target_party_id, jMsg, TYPE_AndruavMessage_SDR_ACTION,  false);
}


void CSDR_Facade::sendSignalAlert(const std::string&target_party_id, const double frequency, const double frequency_signal_value)
{
    /*
        la          : latitude   [degE7]
        ln          : longitude  [degE7]
        a           : absolute altitude
        r           : relative altitude
    */
    
    de::sdr::CSDRMain& cSDRMain = de::sdr::CSDRMain::getInstance();

    ANDRUAV_UNIT_LOCATION& location_info = cSDRMain.getUnitLocationInfo();

    Json_de message=
    {
        {"a", SDR_ACTION_TRIGGER},
        {"la", cSDRMain.getLatitude() },        // latitude   [degE7]
        {"ln", cSDRMain.getLongitude()},        // longitude  [degE7]
        {"r", frequency},                       // frequency
        {"v", frequency_signal_value}           // frequency signal value
        
    };

    if (location_info.is_valid && (location_info.altitude != INT32_MIN)) 
    {
        message["A"] = location_info.altitude;
    }

#ifdef DEBUG
    std::cout <<  "SDR_ACTION_TRIGGER:" << std::to_string(frequency) << std::endl;
#endif

    m_module.sendJMSG (target_party_id, message, TYPE_AndruavMessage_SDR_ACTION, false);
}

void CSDR_Facade::sendLocationInfo (const std::string&target_party_id) const 
{
    /*
        la          : latitude   [degE7]
        ln          : longitude  [degE7]
        a           : absolute altitude
        r           : relative altitude
    */
    
    de::sdr::CSDRMain& cSDRMain = de::sdr::CSDRMain::getInstance();

    if (!cSDRMain.getSendingLocation()) return ;

    Json_de message=
    {
        {"la", cSDRMain.getLatitude() * 1e-7},          // latitude   [degE7]
        {"ln", cSDRMain.getLongitude() * 1e-7},         // longitude  [degE7]
        {"a",  cSDRMain.getAbsoluteAltitude()==INT32_MIN?0:cSDRMain.getAbsoluteAltitude() * 0.001},         // absolute altitude in mm
        {"r",  cSDRMain.getRelativeAltitude()==INT32_MIN?0:cSDRMain.getRelativeAltitude() * 0.001},         // relative altitude in mm
        {"y", 0},                                       // yaw in cdeg
        {"3D",0},
        {"SC",0},
        {"p",0}
    };

    m_module.sendJMSG (target_party_id, message, TYPE_AndruavMessage_GPS, false);
}


void CSDR_Facade::sendSpectrumResultInfo (const std::string&target_party_id, const double frequency_min, const float frequency_step, const uint64_t number_of_data , const float * result) const 
{
    Json_de msg_cmd = {};
    const uint64_t now = get_time_usec();
    // the message is big so few more characters to make it more readable is OK.
    msg_cmd["fcm"]  = frequency_min;
    msg_cmd["fcst"] = frequency_step;
    msg_cmd["tim"]  = now;

        
    m_module.sendBMSG ("", (char *)result, number_of_data * sizeof(float), TYPE_AndruavMessage_SDR_SPECTRUM, false, msg_cmd);
}



        

