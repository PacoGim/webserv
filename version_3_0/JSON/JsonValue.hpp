#ifndef JSON_VALUE_HPP
#define JSON_VALUE_HPP

#include <string>
#include <vector>

bool isValidNumber(const std::string &str);
bool isInvalidPath(const std::string &str);

enum JsonType
{
	JSON_NULL,
	JSON_BOOL,
	JSON_NUMBER,
	JSON_STRING,
	JSON_ARRAY,
	JSON_OBJECT
};

class JsonValue
{
public:
	virtual ~JsonValue() {}
	virtual JsonType type() const = 0;
	virtual bool isEquals(const JsonValue *other) const = 0;
	virtual const JsonValue *get(const std::string &path) const
	{
		if (path == "")
			return (this);
		throw std::string(path + " undefined");
	}

	template <typename T>
	T getValue(const std::string &key) const;

	template <typename T>
	T getValue(const std::string &key,const T &def) const;

	template <typename R>
	std::vector<R> getArrayValue(const std::string &key) const;

	template <typename R>
	std::vector<R> getArrayValue(const std::string &key, const R defaultValue) const;
};

#include "JsonImp/JsonArray.hpp"

template <typename R>
std::vector<R> JsonValue::getArrayValue(const std::string &key) const
{
	const JsonValue *array_type = get(key);
	if (array_type->type() != JSON_ARRAY)
		throw std::string(key + " bad type");
	const JsonArray *array_value = static_cast<const JsonArray *>(array_type);
	std::vector<R> result;
	for (size_t i = 0; i < array_value->values.size(); i++)
	{
		try
		{
			result.push_back(array_value->values[i]->getValue<R>(""));
		}
		catch (const std::string &msg)
		{
			throw(key + "." + msg);
		}
	}
	return (result);
}

template <typename R>
std::vector<R> JsonValue::getArrayValue(const std::string &key, const R defaultValue) const
{
	std::vector<R> result;
	try
	{
		const JsonValue *array_type = get(key);
		if (array_type->type() != JSON_ARRAY)
			throw std::string(key + " bad type");

		const JsonArray *array_value = static_cast<const JsonArray *>(array_type);

		if (array_value->values.size() == 0)
		{
			result.push_back(defaultValue);
			return (result);
		}

		for (size_t i = 0; i < array_value->values.size(); i++)
		{
			result.push_back(array_value->values[i]->getValue<R>(""));
		}
	}
	catch (const std::string &e)
	{
		result.clear();
		result.push_back(defaultValue);
	}
	return (result);
}

#endif
