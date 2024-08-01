#include <iostream>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <sys/time.h>
#include <vector>
#include <sstream>
#include <regex>

#include <iostream>
#include <iomanip>

#include "colors.hpp"
#include "helpers.hpp"



uint64_t convertMACToInteger(const std::string& mac) {
    uint64_t mac_num = 0;
    for (int i = 0; i < 5; i++) {
        std::string octet = mac.substr(i * 3, 2);
        mac_num = (mac_num << 8) | std::stoul(octet, nullptr, 16);
    }
    return mac_num;
}

/**
 * @brief create max address with or without :
 * 
 * @param mac_address_bytes 
 * @param add_colon
 * @return std::string 
 */
std::string formatMacAddress(const std::vector<uint8_t>& mac_address_bytes, const bool add_colon) {
    std::stringstream bssid;

    bssid << std::setw(2) << std::setfill('0') << std::hex; 
    for (int i = 0; i < mac_address_bytes.size(); i++) {
        if (add_colon && (i > 0)) {
            bssid << ":";
        }
        bssid << static_cast<int>(mac_address_bytes[i]);
    }
    #ifdef DDEBUG
        std::cout << "MACID:" << bssid.str() << std::endl;
    #endif
    return bssid.str();
}

std::string removeColons(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c != ':') {
            result += c;
        }
    }
    return result;
}


std::vector<uint8_t> convertMacToBytes(const std::string& macAddress) {
    std::regex mac_regex("([a-fA-F0-9]{2}):([a-fA-F0-9]{2}):([a-fA-F0-9]{2}):([a-fA-F0-9]{2}):([a-fA-F0-9]{2}):([a-fA-F0-9]{2})");
    std::smatch match;
    std::vector<uint8_t> binaryBytes;

    if (std::regex_match(macAddress, match, mac_regex)) {
        std::transform(match.begin() + 1, match.end(), std::back_inserter(binaryBytes),
            [](const std::string& hex) { return static_cast<uint8_t>(std::stoi(hex, nullptr, 16)); });
    }

    return binaryBytes;
}

std::string get_time_string()
{
  struct timeval _time_stamp;
  gettimeofday(&_time_stamp, NULL);

  // Convert the timestamp to a time_t value
  time_t time_t_value = _time_stamp.tv_sec;

  // Convert the time_t value to a tm struct
  struct tm *tm = localtime(&time_t_value);

  // Format the time string
  char time_string[20];
  strftime(time_string, sizeof(time_string), "%Y_%m_%d_%H_%M_%S", tm);

  return time_string;
}

uint64_t get_time_usec()
{
	static struct timeval _time_stamp;
	gettimeofday(&_time_stamp, NULL);
	return _time_stamp.tv_sec*1000000 + _time_stamp.tv_usec;
}



void time_register(uint64_t& time_box)
{
	time_box =  get_time_usec();
}


bool time_passed_usec(const uint64_t& time_box, const uint64_t diff_usec)
{
	const u_int64_t now =  get_time_usec();
    return ((now - time_box) >= diff_usec);
}

bool time_less_usec(const uint64_t& time_box, const uint64_t diff_usec)
{
	const u_int64_t now =  get_time_usec();
    return ((now - time_box) <= diff_usec);
}


bool time_passed_register_usec(uint64_t& time_box, const uint64_t diff_usec)
{
	const u_int64_t now =  get_time_usec();
    const bool passed = ((now - time_box) >= diff_usec);
    if (passed) time_box = now;

    return passed;
}


int wait_time_nsec (const time_t& seconds, const long& nano_seconds)
{
	struct timespec _time_wait, tim2;
	_time_wait.tv_sec = seconds;
	_time_wait.tv_nsec = nano_seconds;
	
	return nanosleep(&_time_wait, &tim2);
}

std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), 
                // static_cast<int(*)(int)>(std::tolower)         // wrong
                // [](int c){ return std::tolower(c); }           // wrong
                // [](char c){ return std::tolower(c); }          // wrong
                   [](unsigned char c){ return std::tolower(c); } // correct
                  );
    return s;
}



std::vector<std::string> split_string_by_delimeter(const std::string& str, const char& delimeter)
{
    auto result = std::vector<std::string>{};
    auto ss = std::stringstream{str};

    for (std::string line; std::getline(ss, line, delimeter);)
        result.push_back(line);

    return result;
}



std::vector<std::string> split_string_by_newline(const std::string& str)
{
    return split_string_by_delimeter (str, '\n');
}


/**
 * @brief 
 * Remove comments from strings
 * http://www.cplusplus.com/forum/beginner/163419/
 * @param prgm 
 * @return std::string 
 */
std::string removeComments(std::string prgm) 
{ 
    int n = prgm.length(); 
    std::string res; 
  
    // Flags to indicate that single line and multpile line comments 
    // have started or not. 
    bool s_cmt = false; 
    bool m_cmt = false; 
  
  
    // Traverse the given program 
    for (int i=0; i<n; i++) 
    { 
        // If single line comment flag is on, then check for end of it 
        if (s_cmt == true && prgm[i] == '\n') 
            s_cmt = false; 
  
        // If multiple line comment is on, then check for end of it 
        else if  (m_cmt == true && prgm[i] == '*' && prgm[i+1] == '/') 
            m_cmt = false,  i++; 
  
        // If this character is in a comment, ignore it 
        else if (s_cmt || m_cmt) 
            continue; 
  
        // Check for beginning of comments and set the approproate flags 
        else if (prgm[i] == '/' && prgm[i+1] == '/') 
            s_cmt = true, i++; 
        else if (prgm[i] == '/' && prgm[i+1] == '*') 
            m_cmt = true,  i++; 
  
        // If current character is a non-comment character, append it to res 
        else  res += prgm[i]; 
    } 
    return res; 
} 


/**
 * @brief Get the linux machine id object
 * 
 * @return std::string 
 */
std::string get_linux_machine_id ()
{
    FILE *f = fopen("/etc/machine-id", "r");
	if (!f) {
		return std::string();
	}
    char line[256]; 
    memset (line,0,255);
    char * read = fgets(line, 256, f);
    if (read!= NULL)
    {
        line[strlen(line)-1]=0; // remove "\n" from the read
        return std::string(line);
    }
    else
    {
        return std::string("");
    }
}


bool is_ascii(const signed char *c, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if(c[i] < 0) return false;
  }
  return true;
}


/**
 * @brief 
 *  returns true if field exist and of specified type
 * @param message Json_de object
 * @param field_name requested field name
 * @param field_type specified type
 * @return true if found and of specified type
 * @return false 
 */
bool validateField (const Json_de& message, const char *field_name, const Json_de::value_t& field_type)
{
    if (
        (message.contains(field_name) == false) 
        || (message[field_name].type() != field_type)
        ) 
    
        return false;

    return true;
}
