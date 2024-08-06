#ifndef SDR_FACADE_H_
#define SDR_FACADE_H_

#include "../de_common/de_facade_base.hpp"

namespace de
{
namespace sdr
{
    class CSDR_Facade : public de::comm::CFacade_Base
    {
        public:
            //https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
            static CSDR_Facade& getInstance()
            {
                static CSDR_Facade instance;

                return instance;
            }

            CSDR_Facade(CSDR_Facade const&)         = delete;
            void operator=(CSDR_Facade const&)      = delete;

        
            // Note: Scott Meyers mentions in his Effective Modern
            //       C++ book, that deleted functions should generally
            //       be public as it results in better error messages
            //       due to the compilers behavior to check accessibility
            //       before deleted status

        private:

            CSDR_Facade()
            {
            };

        public:
            
            ~CSDR_Facade ()
            {
                
            };
                

        public:
        
            void API_SDRInfo (const std::string& target_party_id) const;
            void API_SendSDRDrivers (const std::string& target_party_id) const;
            void sendLocationInfo (const std::string&target_party_id) const;

        
        protected:

            
    };
}
}

#endif
