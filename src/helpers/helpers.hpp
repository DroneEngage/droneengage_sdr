#ifndef HELPERS_H_
#define HELPERS_H_

#include <iostream>
#include <cctype>
#include <algorithm>
#include <sys/time.h>
#include <vector>
#include <sstream>
#include <math.h>

#include "json_nlohmann.hpp"
using Json_de = nlohmann::json;


#define SEC_M500   500000l
#define SEC_1     1000000l
#define SEC_2     2000000l
#define SEC_3     3000000l
#define SEC_4     4000000l
#define SEC_5     5000000l
#define SEC_6     6000000l
#define SEC_7     7000000l
#define SEC_8     8000000l
#define SEC_9     9000000l
#define SEC_10   10000000l
#define SEC_15   15000000l
#define SEC_20   20000000l
#define SEC_30   30000000l


uint64_t convertMACToInteger(const std::string& mac);
std::string formatMacAddress(const std::vector<uint8_t>& mac_address_bytes, const bool add_colon);
std::string removeColons(const std::string& input);
std::vector<uint8_t> convertMacToBytes(const std::string& macAddress);

std::string get_time_string();

uint64_t get_time_usec();

void time_register(uint64_t& time_box);

bool time_passed_usec(const uint64_t& time_box, const uint64_t diff_usec);

bool time_less_usec(const uint64_t& time_box, const uint64_t diff_usec);

bool time_passed_register_usec(uint64_t& time_box, const uint64_t diff_usec);
int wait_time_nsec (const time_t& seconds, const long& nano_seconds);


std::string str_tolower(std::string s);

std::vector<std::string> split_string_by_delimeter(const std::string& str, const char& delimeter);

std::vector<std::string> split_string_by_newline(const std::string& str);

std::string removeComments(std::string prgm);

extern bool is_ascii(const signed char *c, size_t len);

extern bool validateField (const Json_de& message, const char *field_name, const Json_de::value_t& field_type);

extern std::string get_linux_machine_id ();


template <typename T> inline constexpr
int signum(T x, std::false_type is_signed) {
    return T(0) < x;
}

template <typename T> inline constexpr
int signum(T x, std::true_type is_signed) {
    return (T(0) < x) - (x < T(0));
}

template <typename T> inline constexpr
int signum(T x) {
    return signum(x, std::is_signed<T>());
}


#endif