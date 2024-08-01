#ifndef SDR_DRIVER_H_
#define SDR_DRIVER_H_
#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>
#include <vector>
#include <string>
#include <rtaudio/RtAudio.h>
#include <liquid/liquid.h>
#include <fftw3.h>
#include <mutex>


namespace de
{
namespace sdr
{

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6


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


typedef struct DE_SDR_RX{
	double fc;
	double f;
	double sample_rate;
	long size;
	double faudio;
	double bw;
	
	class cStack *cs;
	
	int audioOut;
	
	int decode_mode;
	
	struct Filters fs;
	
	double gain;
	
	int iprint;
	
	double dummy;
	int last;
	std::mutex mutexo;
	std::mutex mutex1;
	std::mutex mutexa;

} DE_SDR_RX;

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

            bool init(const int decode_mode);
            std::vector<RtAudio::DeviceInfo> listDevices();
            bool openStream(const int device_index);
            void startStreaming();
            
            bool openSDR(const std::string driver = "driver=soapyMiri");
            bool closeSDR();
            

        public:

            inline float getFrequencyCenter()
            {
                return m_sdr_rx.fc;
            }

            inline float getFrequency()
            {
                return m_sdr_rx.f;
            }

            inline float getSampleRate()
            {
                return m_sdr_rx.sample_rate;
            }

            inline float getGain()
            {
                return m_sdr_rx.gain;
            }

            inline float getDecodeMode()
            {
                return m_sdr_rx.decode_mode;
            }


            inline float getBandWidth()
            {
                return m_sdr_rx.bw;
            }

        private:

            int center_fft(fftw_complex *out,int N) const;


        private:

            DE_SDR_RX m_sdr_rx;
            RtAudio m_dac;
            std::vector<RtAudio::DeviceInfo> m_devices;
            SoapySDR::Device *m_sdr = NULL;
            SoapySDR::Stream *m_rxStream = NULL;

    };
}
}

#endif