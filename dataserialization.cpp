#include <unistd.h>
#include <stdio.h>
#include <random>
#include <opendht.h>
#include <sstream>

/* Primary data structure for files 
* id: The corresponding unique id of the file, see uuid.cpp
* content: content of the file
* isExists: determines if file exists. Delete sets isExists = 0 */
class valueTriple {
	public:
		unsigned long long id;
		std::string content;
		bool isExists;

		/* Constructors */
		valueTriple();
		valueTriple(unsigned long long id, std::string content, bool isExists);
		// implements msgpack-c serialization methods
		// MSGPACK_DEFINE(id, content);
};

valueTriple::valueTriple() {

}

valueTriple::valueTriple(unsigned long long id, std::string content, bool isExists) {
	this->id = id;
	this->content = content;
	this->isExists = isExists;
}

/* Prepares a valueTriple object to be put into the DHT. Converts
   values to string with format id##isExists##content */
std::string parseOutgoingDHT(valueTriple data) {
	std::stringstream ss;
	ss << std::to_string(data.id) << "##" << std::to_string(data.isExists) << "##" << data.content;

	return ss.str();
}

/* Converts a (DHT) string with format id##isExists##content
   to a valueTriple object with correct values */ 
valueTriple parseIncomingDHT(std::string data) {
	valueTriple dataObject;

	std::string id = data.substr(0, data.find("##"));

	data.erase(0, data.find("##") + 2);
	std::string isExists = data.substr(0, data.find("##"));

	data.erase(0, data.find("##") + 2);
	std::string content = data;

	dataObject.id = std::stoull(id);
	dataObject.isExists = (isExists == "1");
	dataObject.content = content;

	return dataObject;
}

int main() {
	/* For test purposes */
	std::cout << "Testing function parseOutgoingDHT: << std::endl;
	valueTriple data(42, "dogs", 1);
	
	std::string newData = parseOutgoingDHT(data);
	std::cout << newData << std::endl;
	
	std::cout << "Testing function parseIncomingDHT: << std::endl;
	std::string dataString = "43##1##cats";
	
	valueTriple dataObject = parseIncomingDHT(dataString);
	
	std::cout << "id: " << dataObject.id << std::endl;
	std::cout << "isExists: " << dataObject.isExists << std::endl;
	std::cout << "content: " << dataObject.content << std::endl;
	
	return 0;
}
