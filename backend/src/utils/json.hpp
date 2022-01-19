#ifndef JSON_H
#define JSON_H
#include <jsoncons/json.hpp>

/**
* Class for parsing and creating Json
*/
class Json {


public:
	enum Type {
		ANNY,
		ARRAY,
		BOOL,
		NUMBER,
		NONE,
		OBJECT,
		STRING
	};

	Json();
	Json(const Json &other);
	Json(std::string json_str);

	/**
	* Check to see if json contains item of given type
	* @param key: the key to the item to be checked for
	* @param type: the Json::Type to check for
	*	if none given this will default to ANNY and 
	*	will only check that the key exists
	* @return: true if key exists and is of given type
	*	false if key does not exist or does not match type
	*/
	bool has(std::string key, Type type = Type::ANNY);
	
	/**
	* Extract boolean value from Json
	* This assumes that the json item this parser currently holds
	* is a Json Object
	*	thows ParsingError if:
			not holding a Json obj or
	*		key of type bool is not found
	* @param key: the key to get value for
	* @return: the boolean value for given key
	*/
	bool get_bool(std::string key);

	/**
	* Extract string value from Json
	* This assumes that the json item this parser currently holds
	* is a Json Object
	*	thows ParsingError if:
			not holding a Json obj or
	*		key of type string is not found
	* @param key: the key to get value for
	* @return: the string value for given key
	*/
	std::string get_string(std::string key);

	/**
	* Extract string value from Json
	* This assumes that the json item this parser currently holds
	* is a Json Object
	*	thows ParsingError if:
			not holding a Json obj or
	*		key of type obj is not found
	* @param key: the key to get value for
	* @return: a new instance of this class containing the json object
	*/
	Json get_obj(std::string key);

	/**
	* Extract numeric value from Json
	* This assumes that the json item this parser currently holds
	* is a Json Object
	*	thows ParsingError if:
			not holding a Json obj or
	*		key with numeric value is not found
	* @param key: the key to get value for
	* @return: the numeric value for given key
	*/
	double get_number(std::string key);
	
	/**
	* Extract string value from Json
	* This assumes that the json item this parser currently holds
	* is a Json Object
	*	thows ParsingError if:
			not holding a Json obj or
	*		key of type array is not found
	* @param key: the key to get value for
	* @return: a new instance of this class containing the json array
	*/
	Json get_array(std::string key);

	/**
	* Get numeric value at index from Json Array
	* This assumes that the json item this parser currently holds
	* is a Json Array
	*	thows ParsingError if:
			not holding a Json Array or
			index out of bounds
	*		item at index is not numeric
	* @param index: integer index
	* @return: the numeric value for given index
	*/
	double number_at(int index);

	/**
	* Get string value at index from Json Array
	* This assumes that the json item this parser currently holds
	* is a Json Array
	*	thows ParsingError if:
			not holding a Json Array or
			index out of bounds
	*		item at index is not a string
	* @param index: integer index
	* @return: the string value for given index
	*/
	std::string string_at(int index);

	/**
	* Get json object at index from Json Array
	* This assumes that the json item this parser currently holds
	* is a Json Array
	*	thows ParsingError if:
			not holding a Json Array or
			index out of bounds
	*		item at index is not numeric
	* @param index: integer index
	* @return: a new instance of this class containing the json object
	*/
	Json obj_at(int index);

	/**
	* Get boolean value at index from Json Array
	* This assumes that the json item this parser currently holds
	* is a Json Array
	*	thows ParsingError if:
			not holding a Json Array or
			index out of bounds
	*		item at index is not numeric
	* @param index: integer index
	* @return: bool
	*/
	bool bool_at(int index);

	/**
	* Converts the current json item to a string
	* @param should_beutify: (optional) default to false
	*	if true add newlines and tabs to the string
	* @return: json string
	*/
	std::string to_string(bool should_beutify = false);

	/**
	* The size of the json obj or array held by this class
	* @return: int representing the size
	*/
	int get_size();

	/**
	* Get the type of the value found with given key
	* This assumes the json item this class holds is a Json Object
	*	throws ParsingError if:
	*		key not presint in json object
	* @param key: the key for the item in question
	* @return: the Json::Type of the value found at key
	*/
	Json::Type get_type(std::string key);

	/**
	* Gets the type of the json item this class currently holds
	* @return: the Json::Type(OBJECT or ARRAY)
	*/
	Json::Type get_type();


private:
	Json(jsoncons::json json_obj);
	jsoncons::json json_obj;

	/**
	* Get the type of the given item
	* @param item: the item to get the type for
	* @return: Json::Type
	*/
	Json::Type get_type(jsoncons::json item);

	/**
	* Validates that the current json item held is a Json Array
	* thows ParsingError if not
	*/
	void validate_is_array();

	/**
	* Validates that the current json item held is a Json Object
	* throws ParsingError if not
	*/
	void validate_is_obj();

	/**
	* Validates that the given index is within the bounds of the current Json Array
	* throws ParsingError if not
	*/
	void validate_bounds(int index);

};

class JsonError : public std::exception {};

class ParsingError : public JsonError {
public:
	ParsingError();
	ParsingError(std::string error_msg) { error = "Error: " + error_msg; }
	virtual char const* what() const noexcept { return error.c_str(); }
private:
	std::string error = "Undefined Json Parsing Error";
};
#endif // !JSON_H