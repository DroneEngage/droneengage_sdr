#ifndef SDR_PARSER_H_
#define SDR_PARSER_H_

#include "../helpers/json_nlohmann.hpp"
using Json_de = nlohmann::json;

#include "sdr_facade.hpp"

namespace de
{
namespace sdr
{

    /**
     * @brief This class parses messages received via communicator and executes it.
     * 
     */
    class CSDRParser
    {
        public:

            static CSDRParser& getInstance()
            {
                static CSDRParser instance;

                return instance;
            }

            CSDRParser(CSDRParser const&)           = delete;
            void operator=(CSDRParser const&)       = delete;

        
        private:

            CSDRParser() 
            {

            }

            
        public:
            
            ~CSDRParser ()
            {

            }
        
        public:

            void parseMessage (Json_de &andruav_message, const char * message, const int & message_length);
            
        protected:
            void parseRemoteExecute (Json_de &andruav_message);
   

        private:
            de::sdr::CSDR_Facade& m_sdr_facade = de::sdr::CSDR_Facade::getInstance();
    };

}
}

#endif