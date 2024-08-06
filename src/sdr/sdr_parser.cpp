#include "../global.hpp"
#include "../helpers/helpers.hpp"
#include "sdr_parser.hpp"
#include "sdr_facade.hpp"
#include "sdr_driver.hpp"

using namespace de::sdr;


std::thread g;

/// @brief Parses & executes messages received from uavos_comm"
/// @param parsed JSON message received from uavos_comm 
/// @param full_message 
/// @param full_message_length 
void CSDRParser::parseMessage (Json_de &andruav_message, const char * full_message, const int & full_message_length)
{
    const int messageType = andruav_message[ANDRUAV_PROTOCOL_MESSAGE_TYPE].get<int>();
    bool is_binary = !(full_message[full_message_length-1]==125 || (full_message[full_message_length-2]==125));   // "}".charCodeAt(0)  IS TEXT / BINARY Msg  
    

    uint32_t permission = 0;
    if (validateField(andruav_message, ANDRUAV_PROTOCOL_MESSAGE_PERMISSION, Json_de::value_t::number_unsigned))
    {
        permission =  andruav_message[ANDRUAV_PROTOCOL_MESSAGE_PERMISSION].get<int>();
    }

    bool is_system = false;
    if ((validateField(andruav_message, ANDRUAV_PROTOCOL_SENDER, Json_de::value_t::string)) && (andruav_message[ANDRUAV_PROTOCOL_SENDER].get<std::string>().compare(SPECIAL_NAME_SYS_NAME)==0))
    {   // permission is not needed if this command sender is the communication server not a remote GCS or Unit.
        is_system = true;
    }

    if (messageType == TYPE_AndruavMessage_RemoteExecute)
    {
        parseRemoteExecute(andruav_message);

        return ;
    }

    else
    {
        Json_de cmd = andruav_message[ANDRUAV_PROTOCOL_MESSAGE_CMD];
        
        switch (messageType)
        {
            case TYPE_AndruavMessage_ID:
            {
            }
            break;

            case TYPE_AndruavMessage_SDR_ACTION:
            {
/**
                * @brief This is a general purpose message 
                * 
                * a: P2P_ACTION_ ... commands
                * 
                */

                const Json_de cmd = andruav_message[ANDRUAV_PROTOCOL_MESSAGE_CMD];
        
                if (!cmd.contains("a") || !cmd["a"].is_number_integer()) return ;
                    
                CSDRDriver& cSDRDriver  = CSDRDriver::getInstance();                    

                switch (cmd["a"].get<int>())
                {
                    case SDR_ACTION_CONNECT:
                    {
                        cSDRDriver.openSDR();
                    }
                    break;

                    case SDR_ACTION_DISCONNECT:
                    {
                        cSDRDriver.closeSDR();
                    }
                    break;

                    case SDR_ACTION_SET_CONFIG:
                    {
                        std::cout << cmd.dump() << std::endl;
                        if (validateField(cmd,"i", Json_de::value_t::number_unsigned))
                        {
                            cSDRDriver.setSDRDriverIndex(cmd["i"].get<int>());
                        }

                        if (cmd.contains("fc"))
                        {
                            cSDRDriver.setFrequencyCenter(cmd["fc"].get<double>());
                        }

                        if (cmd.contains("f"))
                        {
                            cSDRDriver.setFrequency(cmd["f"].get<double>());
                        }

                        if (cmd.contains("b"))
                        {
                            cSDRDriver.setBandWidth(cmd["b"].get<double>());
                        }
                        
                        if (cmd.contains("g"))
                        {
                            cSDRDriver.setGain(cmd["g"].get<double>());
                        }

                        if (cmd.contains("s"))
                        {
                            cSDRDriver.setSampleRate(cmd["s"].get<double>());
                        }

                        if (cmd.contains("m"))
                        {
                            cSDRDriver.setDecodeMode(cmd["m"].get<double>());
                        }

                        // broadcast updated info.
                        CSDRDriver::getInstance().connect();

                        CSDR_Facade::getInstance().API_SDRInfo(std::string(""));
                        CSDR_Facade::getInstance().sendLocationInfo(std::string(""));
                    }
                    break;

                    case SDR_ACTION_READ_DATA:
                    {
                        //cSDRDriver.startStreaming();
                        g = std::thread {[&](){ 
                                cSDRDriver.startStreaming();
                                g.detach();}};
                    }
                    break;
                    
                    default:
                    {

                    }
                    break;
                }
            }
            break;

            case TYPE_AndruavMessage_SDR_STATUS:
            {
                //SDR_ACTION_SET_CONFIG
            }
            break;

            default:
            {

            }
            break;
        }

    }

    UNUSED(is_binary);
}

/**
 * @brief part of parseMessage that is responsible only for
 * parsing remote execute command.
 * 
 * @param andruav_message 
 */
void CSDRParser::parseRemoteExecute (Json_de &andruav_message)
{
    const Json_de cmd = andruav_message[ANDRUAV_PROTOCOL_MESSAGE_CMD];
    
    if (!validateField(cmd, "C", Json_de::value_t::number_unsigned)) return ;
                
    uint32_t permission = 0;
    if (validateField(andruav_message, ANDRUAV_PROTOCOL_MESSAGE_PERMISSION, Json_de::value_t::number_unsigned))
    {
        permission =  andruav_message[ANDRUAV_PROTOCOL_MESSAGE_PERMISSION].get<int>();
    }

    UNUSED (permission);
    
    bool is_system = false;
     
    if ((validateField(andruav_message, ANDRUAV_PROTOCOL_SENDER, Json_de::value_t::string)) && (andruav_message[ANDRUAV_PROTOCOL_SENDER].get<std::string>().compare(SPECIAL_NAME_SYS_NAME)==0))
    {   // permission is not needed if this command sender is the communication server not a remote GCS or Unit.
        is_system = true;
    }
    const int remoteCommand = cmd["C"].get<int>();
    std::cout << "cmd: " << remoteCommand << std::endl;
    
    switch (remoteCommand)
    {
        case TYPE_AndruavMessage_SDR_INFO:
            CSDR_Facade::getInstance().API_SDRInfo(andruav_message[ANDRUAV_PROTOCOL_SENDER].get<std::string>());
        break;

        case TYPE_AndruavMessage_SDR_ACTION:
        {
            const int subCommand = cmd["a"].get<int>();

            switch (subCommand)
            {
                case SDR_ACTION_LIST_SDR_DEVICES:
                    CSDR_Facade::getInstance().API_SendSDRDrivers(andruav_message[ANDRUAV_PROTOCOL_SENDER].get<std::string>());
                    break;
                
                default:
                    break;
            }
        }
        break;

        case TYPE_AndruavMessage_SDR_STATUS:
        break;
    }
}