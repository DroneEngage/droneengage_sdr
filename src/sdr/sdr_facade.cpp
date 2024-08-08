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
            {"c" , cSDRDriver.getStatus()},
            {"fc", cSDRDriver.getFrequencyCenter()},
            {"f" , cSDRDriver.getFrequency()},
            {"s" , cSDRDriver.getSampleRate()},
            {"g" , cSDRDriver.getGain()},
            {"m" , cSDRDriver.getDecodeMode()},
            {"b" , cSDRDriver.getBandWidth()},
            {"i" , cSDRDriver.getSDRDriverIndex()}
        };

    #ifdef DDEBUG
        std::cout << "XXXXXXXXXXXXXXXXXXXXX" << jMsg.dump() << std::endl;
    #endif
    
    m_module.sendJMSG (target_party_id, jMsg, TYPE_AndruavMessage_SDR_INFO,  false);
    
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
        int driver_index = -1;
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

    m_module.sendJMSG (target_party_id, jMsg, TYPE_AndruavMessage_SDR_STATUS,  false);
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

    Json_de message=
    {
        {"la", cSDRMain.getLatitude()},         // latitude   [degE7]
        {"ln", cSDRMain.getLongitude()},        // longitude  [degE7]
        {"a", 0},                               // absolute altitude in mm
        {"r", 0},                               // relative altitude in mm
        {"y", 0},                               // yaw in cdeg
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

