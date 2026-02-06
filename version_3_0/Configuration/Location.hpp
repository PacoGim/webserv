#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <string>
#include "../Http/Method.hpp"

#define MAX_BODY (1024 * 1024)
#define LOCATION_BAD_TYPE "location bad type"
#define LOCATION_URI_UNDEFINED "uri undefined"
#define LOCATION_URI_BAD_TYPE "uri bad type"

class JsonValue;

class Location
{
public:
	std::string uri;
	std::string path;
	std::string default_index;
	std::string cgi;
	int max_body;
	int enable_listing;
	Http::AllowedMethods methods;

	Location(const std::string &uri, const std::string &path, int max_body, int enable_listing, int methods);
	Location(void);

	explicit Location(const JsonValue *location);
	std::string view(void) const;
};

#endif
