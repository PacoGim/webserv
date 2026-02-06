#ifndef VOGO_WEBSERV_STRING_HPP
#define VOGO_WEBSERV_STRING_HPP

#include <string>
#include <vector>
#include <stdint.h>

struct String
{
   static const uint8_t MAX_HEXA_DIGIT;
   static std::string to_upper(const std::string &str);
   static std::string trim(const std::string &str);
   static std::string header_trim(const std::string &str);
   static bool is_header_space(unsigned char c);
   static bool is_blank(const std::string &str);
   static bool if_starts_by(const std::string &str, const std::string &to_find);
   static bool if_ends_by(const std::string &str, const std::string &to_find);
   static std::string escapeHTML(const std::string &str);
   static std::vector<std::string> split(const std::string &s, char delimiter);
   static char convert_to_hex(unsigned int number);
   static void sprint_hexa(char *buffer, size_t chunk_size);
   static void sprint_final_chunk(char *buffer, size_t chunk_size);
   static std::string url_encode(const std::string &value);
   static std::string sanitize_filename(const std::string &filename);
}; // Helper

#endif //VOGO_WEBSERV_STRING_HPP