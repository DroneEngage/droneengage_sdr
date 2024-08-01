#include "../defines.hpp"

#include "../helpers/helpers.hpp"

#include "sdr_facade.hpp"

#include "sdr_driver.hpp"

using namespace de::sdr;

using namespace de::comm;


void CSDR_Facade::API_SDRInfo(const std::string&target_party_id) const
{
    
    //de::andruav_servers::CP2P& cP2P = de::andruav_servers::CP2P::getInstance();
    CSDRDriver& cSDRDriver  = CSDRDriver::getInstance();
    
    Json_de jMsg = 
        {
    //         {"c",  static_cast<int>(me_unit_p2p_info.p2p_connection_type)},
    //         {"a1", static_cast<std::string>(me_unit_p2p_info.address_1)},
    //         {"a2", static_cast<std::string>(me_unit_p2p_info.address_ap)},
    //         {"wc", static_cast<std::uint8_t>(me_unit_p2p_info.wifi_channel)},
    //         {"wp", static_cast<std::string>(me_unit_p2p_info.wifi_password)},

    //         {"pa", static_cast<std::string>(me_unit_p2p_info.parent_address)},
    //         {"pc", static_cast<bool>(me_unit_p2p_info.parent_connection_status)},
    //         {"f",  static_cast<std::string>(me_unit_p2p_info.firmware_version)},
    //         {"lp", static_cast<std::string>(cP2P.getExpectedParentMac())},
            
    //         {"a", static_cast<bool>(me_unit_p2p_info.driver_connected)},
    //         {"o", static_cast<bool>(me_unit_p2p_info.is_p2p_connected)},
    //         {"d", static_cast<bool>(me_unit_p2p_info.is_p2p_disabled)}
            {"c",1},
            {"fc", cSDRDriver.getFrequencyCenter()},
            {"f", cSDRDriver.getFrequency()},
            {"s", cSDRDriver.getSampleRate()},
            {"g", cSDRDriver.getGain()},
            {"d", cSDRDriver.getDecodeMode()},
            {"b", cSDRDriver.getBandWidth()}
        };

    #ifdef DDEBUG
        std::cout << "XXXXXXXXXXXXXXXXXXXXX" << jMsg.dump() << std::endl;
    #endif
    
    m_module.sendJMSG (target_party_id, jMsg, TYPE_AndruavMessage_SDR_INFO,  false);
    
}

