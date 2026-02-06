#include "Location.hpp"
#include "../JSON/Json.hpp"

#include <sstream>
#include <vector>

Location::Location(void)
	: uri("/*"),
	  path("www"),
	  default_index("index.html"),
	  cgi(""),
	  max_body(0),
	  enable_listing(-1),
	methods(0)
{
}

Location::Location(const std::string &uri, const std::string &path, const int max_body, const int enable_listing, const int methods)
	: uri(uri),
	  path(path),
	  default_index(""),
	  cgi(""),
	  max_body(max_body),
	  enable_listing(enable_listing),
	  methods(methods)
{
}

Location::Location(const JsonValue *location) : methods(0) {
	if (location == NULL || location->type() != JSON_OBJECT)
		throw std::string(LOCATION_BAD_TYPE);
	uri = location->getValue<std::string>("uri");
	const std::vector<std::string> methods_str = location->getArrayValue<std::string>("methods", "GET");
	methods = Http::Method::parse_method(methods_str);
	path = location->getValue<std::string>("path", "www");
	default_index = location->getValue<std::string>("default_index", "");
	cgi = location->getValue<std::string>("cgi", "");
	max_body = location->getValue<int>("max_body", -1);

	try
	{
		enable_listing = location->getValue<bool>("enable_listing");
	}
	catch (std::string &e)
	{
		(void) e;
		enable_listing = -1;
	}
}

std::string Location::view(void) const
{
	std::ostringstream oss;

	oss << "          \"uri\": \"" << uri << "\",\n"
		<< "          \"path\": \"" << path << "\",\n"
		<< "          \"default_index\": \"" << default_index << "\",\n"
		<< "          \"cgi\": \"" << cgi << "\",\n"
		<< "          \"maxBody\": " << max_body << ",\n"
		<< "          \"enable_listing\": " << enable_listing << ", \n"
		<< "          \"methods\": [" << Http::Method::to_string(methods) << "]" << std::endl;
	return (oss.str());
}

template <>
Location JsonValue::getValue<Location>(const std::string &key) const
{
	if (this->type() != JSON_OBJECT)
		throw std::string("location bad type");
	if (!key.empty())
		throw std::string("location undefined");
	return (Location(this));
}

#include <cstdio>

template <>
Location JsonValue::getValue<Location>(const std::string &key, const Location &loc) const
{
	if (this->type() != JSON_OBJECT)
		return (loc);
	if (!key.empty())
	{
		printf("\n\n######getValueLocation KEY IS NOT EMPTY %s\n\n", key.c_str());
		return (loc);
	}
	return (Location(this));
}