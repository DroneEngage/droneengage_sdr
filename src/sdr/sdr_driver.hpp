#ifndef SDR_DRIVER_H_
#define SDR_DRIVER_H_
#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>
#include <vector>
#include <string>
#include <liquid/liquid.h>
#include <fftw3.h>
#include <mutex>


namespace de
{
namespace sdr
{


    typedef struct Filters{
        int np;
        ampmodem demodAM;
        freqdem demod;
        msresamp_crcf iqSampler;
        msresamp_rrrf iqSampler2;
        nco_crcf fShift;
        int thread;	
        double amHistory;
    } DE_Filters;


    typedef struct rxStruct{
        double fc;
        double f;
        double sample_rate;
        long size;
        double faudio;
        double bw;
        
        class cStack *cs;
        
        int audioOut;
        
        int decode_mode;
        
        DE_Filters fs;
        
        double gain;
        
        int iprint;
        
        double dummy;
        int last;
        std::mutex mutexo;
        std::mutex mutex1;
        std::mutex mutexa;

    } DE_SDR_RX;

    enum class ENUM_STATE
	{
		NOT_CONNECTED       = 0,
        CONNECTED           = 1,
		STREAMING_ONCE      = 2,
        STREAMING_INTERVALS  = 4,
        ERROR               = 999
	};

    enum class ENUM_DECODING_MODE
	{
		MODE_FM         = 0,
		MODE_NBFM       = 1,
		MODE_AM         = 2,
		MODE_NAM        = 3,
		MODE_USB        = 4,
		MODE_LSB        = 5,
        MODE_CW         = 6
	};






#define NUM_ABUFF 10

#define NUM_DATA_BUFF 10


    class cStack{
    public:
        cStack(DE_SDR_RX *rx);
        ~cStack();
        int pushBuffa(int nbuffer,DE_SDR_RX *rx);
        int popBuffa(DE_SDR_RX *rx);
        int setBuff(DE_SDR_RX *rx);
        DE_SDR_RX *rx;    

        float *buffa[NUM_ABUFF];
        int buffStacka[NUM_ABUFF];

        int bufftopa;
        int bufftop;

    };

    /**
     * @brief 
     * 
     */
    class CSDRDriver
    {
        public:

            static CSDRDriver& getInstance()
            {
                static CSDRDriver instance;

                return instance;
            }

            CSDRDriver(CSDRDriver const&)               = delete;
            void operator=(CSDRDriver const&)           = delete;

        
        private:

            CSDRDriver() 
            {

            }

            
        public:
            
            ~CSDRDriver ()
            {

            }
        

        public:

            bool init();
            void listDevices();
            void startStreamingOnce();
            void startStreaming();
            
            bool openSDR();
            bool closeSDR();
        
            
        public:

            inline ENUM_STATE getStatus() const
            {
                return m_status;
            }

            inline int getSDRDriverIndex() const
            {
                return m_sdr_index;
            }

            inline uint64_t getBars() const
            {
                return m_bars;
            }

            inline void setBars(const uint64_t bars)
            {
                m_bars = bars;
            }

            inline uint64_t getIntervals() const
            {
                return m_intervals;
            }

            inline void setIntervals(const uint64_t intervals)
            {
                m_intervals = intervals;
            }

            inline float getFrequencyCenter() const
            {
                return m_sdr_rx.fc;
            }

            inline void setFrequencyCenter(const float frequency_center)
            {
                m_sdr_rx.fc = frequency_center;
            }


            inline float getFrequency() const
            {
                return m_sdr_rx.f;
            }

            inline void setFrequency(const float frequency)
            {
                m_sdr_rx.f = frequency;
            }


            inline float getSampleRate() const
            {
                return m_sdr_rx.sample_rate;
            }

            inline void setSampleRate(const float sample_rate)
            {
                m_sdr_rx.sample_rate = sample_rate;
            }


            inline float getGain() const
            {
                return m_sdr_rx.gain;
            }

            inline void setGain(const float gain)
            {
                m_sdr_rx.gain = gain;
            }

            
            inline float getDecodeMode() const
            {
                return m_sdr_rx.decode_mode;
            }

            inline void setDecodeMode(const float decode_mode)
            {
                m_sdr_rx.decode_mode = decode_mode;
            }


            inline float getBandWidth() const
            {
                return m_sdr_rx.bw;
            }

            inline void setBandWidth(const float band_width)
            {
                m_sdr_rx.bw = band_width;
            }

            inline std::vector<SoapySDR::Kwargs> get_device_agrs() const 
            {
                return m_device_args;
            }

        public:

            void setSDRDriverIndex(const int  sdr_driver_index);
            void setSDRDriverByName(const std::string sdr_driver);

        private:

            int center_fft(fftw_complex *out,int N) const;

            void setStatus(ENUM_STATE status);



        private:

            DE_SDR_RX m_sdr_rx;
            std::vector<SoapySDR::Kwargs> m_device_args;
            SoapySDR::Device *m_sdr = NULL;
            SoapySDR::Stream *m_rxStream = NULL;

            ENUM_STATE m_status;

            std::string m_sdr_driver;
            int m_sdr_index = 0; 
            float *buff1 = nullptr;
            float *buff2 = nullptr;
            float *buff3 = nullptr;

            fftwf_plan p1;
            fftwf_plan p2;
            size_t m_max_tx_unit;

            uint64_t m_bars = 32;
            uint64_t m_intervals = 0; // no interval
            bool m_exit = false;

            std::mutex m_lock;
    };
}
}

#endif