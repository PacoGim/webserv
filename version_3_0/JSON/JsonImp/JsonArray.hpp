#ifndef JSON_ARRAY_HPP
#define JSON_ARRAY_HPP

#include <vector>
#include "../JsonValue.hpp"

class JsonArray : public JsonValue
{
public:
	std::vector<JsonValue *> values;

	~JsonArray(void);
	JsonType type(void) const;
	bool isEquals(const JsonValue *other) const;
	virtual const JsonValue *get(const std::string &path) const;
};

#endif
