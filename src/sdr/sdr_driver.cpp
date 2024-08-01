
#include "sdr_driver.hpp"

using namespace de::sdr;

int CSDRDriver::center_fft (fftw_complex *out,int N) const 
{
  int i;
  double work;

  //centering the fourier transform
  for(i=0;i<N/2;i++){
    work= out[N/2 +i][0];
    out[N/2 +i][0] = out[i][0];
    out[i][0] = work;

    work= out[N/2 +i][1];
    out[N/2 +i][1] = out[i][1];
    out[i][1] = work;  
  }
  return 0;
}


bool CSDRDriver::init(const int decode_mode)
{

    //rx->cs=new cStack(rx);
	
	m_sdr_rx.decode_mode=MODE_NBFM;
	
	m_sdr_rx.gain=0.0;
	
	m_sdr_rx.fc=100.3e6;
	
	m_sdr_rx.f=100.6e6;
	
	m_sdr_rx.sample_rate=1.3e6;
	
    m_sdr_rx.decode_mode = decode_mode;


	//int device=-2;


    return true;
}


std::vector<RtAudio::DeviceInfo> CSDRDriver::listDevices()
{
    std::vector<RtAudio::DeviceInfo> devices;
    int deviceCount = m_dac.getDeviceCount();

    for (int i = 0; i < deviceCount; i++) {
        try {
            RtAudio::DeviceInfo info = m_dac.getDeviceInfo(i);
            if (info.outputChannels > 0) {
                #ifdef DEBUG
                std::cout << "device =  " << i << " : maximum output  channels = " << info.outputChannels << " Device Name =  " << info.name.c_str() << std::endl;
                #endif
                devices.push_back(info);
            }
        }
        catch (RtAudioError &error) {
            error.printMessage();
            break;
        }
    }

    return devices;

}

/**
 * @brief Sound
 * 
 * @param device_index 
 * @return true 
 * @return false 
 */
bool CSDRDriver::openStream(const int device_index)
{
    // RtAudio::StreamParameters parameters;

    // if (device_index == -1)
    // {
    //     parameters.deviceId = m_dac.getDefaultOutputDevice();
	// }
    // else
    // {
	//     parameters.deviceId = device_index;
	// }
	
    // parameters.nChannels = 1;
	// parameters.firstChannel = 0;
	// unsigned int sampleRate = 48000;
	// unsigned int bufferFrames = 2400;
	
    // try 
    // {
	// 	m_dac.openStream( &parameters, NULL, RTAUDIO_FLOAT32,
	// 					m_sdr_rx.sample_rate, &bufferFrames, &sound, (void *)&m_sdr_rx);
	// 	m_dac.startStream();
	// }
	// catch ( RtAudioError& e ) {
	// 	e.printMessage();
	// 	return false;
	// } 

    return true;
}



bool CSDRDriver::closeSDR()
{
    if (m_sdr == NULL) return true;

    m_sdr->deactivateStream(m_rxStream, 0, 0);

	m_sdr->closeStream(m_rxStream);

	SoapySDR::Device::unmake(m_sdr);
}

bool CSDRDriver::openSDR(const std::string driver )
{
    closeSDR();

    m_sdr = SoapySDR::Device::make(driver);
    if (m_sdr == NULL)
    {
        std::cout << "No device! Found" << std::endl;
        return false;
    }


}


void CSDRDriver::startStreaming()
{

}