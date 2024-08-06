
#include "../de_common/configFile.hpp"
#include "../de_common/messages.hpp"
#include "sdr_facade.hpp"
#include "sdr_driver.hpp"


using namespace de::sdr;

CSDR_Facade& cSDR_Facade  = CSDR_Facade::getInstance();


int setFilters(DE_SDR_RX& rx, DE_Filters& f)
{
    
    float As = 60.0f;
    
    liquid_ampmodem_type mode=LIQUID_AMPMODEM_DSB;
    
    int iflag=0;
    
    if(rx.decode_mode == (int) ENUM_DECODING_MODE::MODE_AM){
        rx.bw=10000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    } else if(rx.decode_mode == (int) ENUM_DECODING_MODE::MODE_NAM){
        rx.bw=5000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    } else if(rx.decode_mode == (int) ENUM_DECODING_MODE::MODE_NBFM){
        rx.bw=12500.0;
    }else if(rx.decode_mode == (int) ENUM_DECODING_MODE::MODE_FM){
        rx.bw=200000.0;
    }else if(rx.decode_mode == (int) ENUM_DECODING_MODE::MODE_USB){   // Above 10 MHZ
        rx.bw=6000.0;
        mode=LIQUID_AMPMODEM_USB;
        iflag=1;
    }else if(rx.decode_mode == (int) ENUM_DECODING_MODE::MODE_LSB){  // Below 10 MHZ
        rx.bw=6000.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }else if(rx.decode_mode == (int) ENUM_DECODING_MODE::MODE_CW){
        rx.bw=3000.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }
    
    double Ratio1 = (float)(rx.bw/ rx.sample_rate);
    
    double Ratio2=(float)(rx.faudio / rx.bw);

    fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
    
    f.demod=freqdem_create(0.5);
    
#ifdef LIQUID_VERSION_4
    f.demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
 #else
    f.demodAM = ampmodem_create(0.5, mode, iflag);
#endif

    f.iqSampler  = msresamp_crcf_create(Ratio1, As);
    
    f.iqSampler2 = msresamp_rrrf_create(Ratio2, As);    
    
    f.fShift = nco_crcf_create(LIQUID_NCO);
    
    f.amHistory=0;
    
    return 0;
    	
}


cStack::cStack(DE_SDR_RX *rxi)
{
    rx=rxi;
    bufftopa=0;
    bufftop=0;
    
    for(int k=0;k<NUM_ABUFF;++k){
    	buffStacka[k]=0;
    	buffa[k]=NULL;
    }
}
cStack::~cStack()
{
	
	for(int k=0;k<NUM_ABUFF;++k){
		if(buffa[k])free((char *)buffa[k]);
		buffa[k]=NULL;
	}
}

int cStack::setBuff(DE_SDR_RX *rx)
{

	
	for(int k=0;k<NUM_ABUFF;++k){
		if(buffa[k])free((char *)buffa[k]);
		buffa[k]=(float *)malloc((size_t)(2*rx->faudio*4*sizeof(float)));
		if(!buffa[k]){
			fprintf(stderr,"10 cMalloc Errror %ld\n",(long)(2*rx->faudio*4));
			return 1;
		}
		memset(buffa[k],0,2*rx->faudio*4*sizeof(float));
		buffStacka[k]=-1;
	}


	return 0;
}

int cStack::pushBuffa(int nbuffer,DE_SDR_RX *rx)
{

	rx->mutexa.lock();
	
    if(bufftopa >= NUM_ABUFF){
        bufftopa=NUM_ABUFF;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_ABUFF;++k){
             if(buffStacka[k] < small2){
             	small2=buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	buffStacka[ks]=nbuffer;
        }
   }else{
    	buffStacka[bufftopa++]=nbuffer;
    }
    
	rx->mutexa.unlock();

	return 0;
}

int cStack::popBuffa(DE_SDR_RX *rx)
{
	int ret;
	
	
	rx->mutexa.lock();
	
	ret=-1;
	
 	if(bufftopa < 1)goto Out;
 	
 	if(bufftopa == 1){
 		ret=buffStacka[0];
 		bufftopa=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<bufftopa;++k){
             if(buffStacka[k] < small2){
             	small2=buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	ret=buffStacka[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<bufftopa;++k)
        	{
        		if(k == ks)continue;
        		buffStacka[kk++]=buffStacka[k];
        	}
        	bufftopa--;
        }
	
	
Out:
	rx->mutexa.unlock();

	return ret;
}


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


void CSDRDriver::setStatus(ENUM_STATE status)
{
    bool changed = (m_status == status);
    m_status = status;

    if (changed)
    {
        cSDR_Facade.API_SDRInfo(std::string(""));
    }
}

void CSDRDriver::connect()
{
    listDevices();
    if (m_status == ENUM_STATE::NOT_CONNECTED)
    {
        openSDR();
    }
}



bool CSDRDriver::init()
{

    //m_sdr_rx.cs=new cStack(rx);
	
	m_sdr_rx.decode_mode = (int) ENUM_DECODING_MODE::MODE_NBFM;
	
	m_sdr_rx.gain = 50.0;
	
	m_sdr_rx.fc = 100.3e6;
	
	m_sdr_rx.f = 100.6e6;
	
	m_sdr_rx.sample_rate = 1.3e6;

    m_sdr_rx.faudio = 48000;
	
    return true;
}


void CSDRDriver::listDevices()
{
    SoapySDR::Kwargs device_arg; 
            
    m_device_args.clear();
    
    m_device_args = SoapySDR::Device::enumerate();
    
    fprintf(stderr,"Number of SDR Devices Found: %ld\n",(long)m_device_args.size());
    
    if(m_device_args.size() < 1) return ;
    

   for(unsigned int k=0;k<m_device_args.size();++k){
		fprintf(stderr,"SDR device =  %ld ",(long)k);
		device_arg = m_device_args[k];
		for (SoapySDR::Kwargs::const_iterator it = device_arg.begin(); it != device_arg.end(); ++it) {
			std::cout << it->first << " = " << it->second << std::endl;
		}
    }
    
    
}


void CSDRDriver::setSDRDriverIndex(const int  sdr_driver_index)
{
    listDevices();

    if (m_device_args.size()<=sdr_driver_index) 
    {
        std::cout << "SDR Driver - Out of index." << std::endl;
        return ;
    }

    m_sdr_index = sdr_driver_index;

    SoapySDR::Kwargs device_arg = m_device_args[m_sdr_index]; 
    
    for (SoapySDR::Kwargs::const_iterator it = device_arg.begin(); it != device_arg.end(); ++it) {
        std::cout << it->first << " = " << it->second << std::endl;
        if (it->first=="driver")
        {
            m_sdr_driver = it->second;
            return ;
        }
	}

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

bool CSDRDriver::openSDR( )
{
    if (!closeSDR())
    {
        cSDR_Facade.sendErrorMessage(std::string(), 0, ERROR_TYPE_ERROR_P2P, NOTIFICATION_TYPE_WARNING, "Could not close SDR.");
        std::cout << "Could not close SDR" << std::endl;
        setStatus(ENUM_STATE::ERROR);
    }

    setStatus(ENUM_STATE::NOT_CONNECTED);
    //listDevices();
    setSDRDriverIndex(m_sdr_index);

    // Open SDR Device.
    std::cout << "m_sdr_driver: " << m_sdr_driver << std::endl;
    m_sdr = SoapySDR::Device::make(m_sdr_driver);
    
    if (m_sdr == NULL)
    {
        std::cout << "No device! Found" << std::endl;
        cSDR_Facade.sendErrorMessage(std::string(), 0, ERROR_TYPE_ERROR_P2P, NOTIFICATION_TYPE_ERROR, "Cannot open SDR Driver");
        setStatus(ENUM_STATE::ERROR);
        return false;
    }
    
    setStatus(ENUM_STATE::CONNECTED);
    cSDR_Facade.sendErrorMessage(std::string(), 0, ERROR_TYPE_ERROR_P2P, NOTIFICATION_TYPE_INFO, "SDR is Active.");

    m_sdr->setFrequency(SOAPY_SDR_RX,0,"RF",m_sdr_rx.fc);
    
    std::cout << "m_sdr_rx.fc:" << m_sdr_rx.fc << std::endl;

    m_sdr->setSampleRate(SOAPY_SDR_RX, 0, m_sdr_rx.sample_rate);

    std::cout << "m_sdr_rx.sample_rate:" << m_sdr_rx.sample_rate << std::endl;
	m_sdr->setGain(SOAPY_SDR_RX, 0, m_sdr_rx.gain); 

	m_sdr_rx.size=m_sdr_rx.sample_rate/20;

    m_rxStream = m_sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, (const std::vector<size_t>)0);


	const int ret = m_sdr->activateStream(m_rxStream, 0, 0, 0); 
    if (ret !=0)
    {
        std::cout << "Could not activate stream." << std::endl;
        setStatus(ENUM_STATE::ERROR);
        return false;
    }
	
	MTU = m_sdr->getStreamMTU(m_rxStream);
    
    std::cout << "MCU: " << MTU << std::endl;

    setFilters(m_sdr_rx, m_sdr_rx.fs);

    float shift=m_sdr_rx.fc - m_sdr_rx.f;

    nco_crcf_set_frequency(m_sdr_rx.fs.fShift,(float) ((2.0 * M_PI) * (((double) abs(shift)) / ((double) m_sdr_rx.sample_rate))));
   

    //create a re-usable buffer for rx samples
	buff1= new float[(int)(m_sdr_rx.sample_rate*sizeof(float)*8)];
	buff2= new float[(int)(m_sdr_rx.sample_rate*sizeof(float)*8)];
	buff3= new float[(int)(m_sdr_rx.sample_rate*sizeof(float)*8)];
	
    #ifdef DDEBUG
	    std::cout << "=====================m_sdr_rx.sample_rate " << m_sdr_rx.sample_rate << std::endl;
    #endif

				
	p1 = fftwf_plan_dft_1d(m_sdr_rx.sample_rate,(fftwf_complex *)buff1, (fftwf_complex *)buff2, FFTW_FORWARD, FFTW_ESTIMATE);
	
	
				
	p2 = fftwf_plan_dft_1d((int)m_sdr_rx.bw,(fftwf_complex *)buff3, (fftwf_complex *)buff1, FFTW_BACKWARD, FFTW_ESTIMATE);

	fprintf(stderr,"Read m_sdr_rx.bw %d\n",(int)m_sdr_rx.bw);
	
    return true;
}


void CSDRDriver::startStreaming()
{
    if (m_status!=ENUM_STATE::CONNECTED) return ;
    try
    {
    long int count=10;

	while (count>0)
	{	
        m_status = ENUM_STATE::STREAMING;

		void *buffs[1];
		float *out;
		int flags=0;
		long long timeNs=0;
		int rec=m_sdr_rx.sample_rate;
		int nout;
		out=buff1;
        
        #ifdef DDEBUG
            std::cout << " ============= rec:" << rec << std::endl;
        #endif

		while(rec > 0){
		    flags=0;
		    buffs[0]=out;
		    nout=rec;
		    if(nout > MTU)nout=MTU;
            
            #ifdef DDEBUG
                std::cout << "=====================nout " << nout << std::endl;
            #endif

			int ret = m_sdr->readStream(m_rxStream, buffs, nout, flags, timeNs, 100000L);
			if(ret < 0){
			   //fprintf(stderr,"Read Error rec %d flags %d count %ld\n",rec,flags,count);
			   continue;
			}
			out += 2*(long long)ret;
			rec -= ret;
		}
        
		
		count++;
		
		for(long int n=0;n<(long int)(m_sdr_rx.sample_rate);++n){
			buff1[n*2] *= pow(-1.0,n);
			buff1[n*2+1] *= pow(-1.0,n);
		}				
	
		
		fftwf_execute(p1);
				
		if(true) { //count == -10){
		
			printf("plot %d sinwave1\n",(int)m_sdr_rx.sample_rate);
			for(long int np=0;np<(long)(m_sdr_rx.sample_rate);++np){
				double v=buff2[np*2]*buff2[np*2]+buff2[np*2+1]*buff2[np*2+1];
				if(v)v=sqrt(v);
				double t=m_sdr_rx.fc-0.5*m_sdr_rx.sample_rate+np;
				printf("%f %f\n",t,v);
			}				
		
			//exit(0);

		}
		count --;
    }   

    setStatus(ENUM_STATE::CONNECTED);
    }
    catch (...)
    {
        std::cout << "ERROR:" << std::endl;
        setStatus(ENUM_STATE::CONNECTED);
    }
    
}