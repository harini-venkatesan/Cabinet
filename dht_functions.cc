/* File contains many print statements to show program progress */

#include <unistd.h>
#include <stdio.h>
#include <random>
#include <opendht.h>
#include <sstream>

#include "functions.h"
#include "file.h"

/* Global dhtrunner */
dht::DhtRunner node;

/* Prepares a file object to be put into the DHT. Converts
   values to string with format id##isExists##content */
std::string parseOutgoingDHT(file data) {
        std::stringstream ss;
        ss << std::to_string(data.id) << "##" << std::to_string(data.isExists) << "##" << data.content;

        return ss.str();
}

/* Converts a (DHT) string with format id##isExists##content
   to a file object */
file parseIncomingDHT(std::string data) {
        file fileObject;

        std::string id = data.substr(0, data.find("##"));

        data.erase(0, data.find("##") + 2);
        std::string isExists = data.substr(0, data.find("##"));

        data.erase(0, data.find("##") + 2);
        std::string content = data;

        fileObject.id = std::stoull(id);
        fileObject.isExists = (isExists == "1");
        fileObject.content = content;

        return fileObject;
}

void dht_init_root() {
        file root_directory(1, ".", true);

        node.put("/", "1");
        node.put("1", parseOutgoingDHT(root_directory));
}

void dht_mkdir(std::string path_name) {
	unsigned long long file_id = idGenerator();
	std::string parent_uuid;

	std::cout << "Entering dht_mkdir...\n\n";

	/* Create new directory */
	std::cout << "Creating new directory on dht...\n";

	file new_directory(file_id, ".", true);
	/* put <(full)path_name, id> */
	node.put(path_name, std::to_string(file_id));
	/* put <id, <id,.>> */
	node.put(std::to_string(file_id), parseOutgoingDHT(new_directory));

	sleep(1);	

	/*  Put new directory into parent path */
	std::cout << "Linking new directory to parent...\n";

	std::string parent_name = parent_path(path_name);
	
	//Get uuid (key) of parent
	node.get(parent_name, [&parent_uuid](const std::vector<std::shared_ptr<dht::Value>>& values) {
		for (const auto& value : values) {
			parent_uuid = asciify((*value).data);
        	}
        	return true;
        });

	sleep(1);

	new_directory.content = file_name(path_name);
	node.put(parent_uuid, parseOutgoingDHT(new_directory));

	sleep(1);

	std::cout << "\nLeaving dht_mkdir...\n\n";
}

void dht_get() {
	/* string for converting name to uuid */
	std::string name2file;
	/* string for storing all file strings in uuid */
	std::vector<std::string> fileStringsVec;
	/* vector that contains the conversion of file strings to data type file */
	std::vector<file> fileVec;

	std::cout << "Entering dht_get...\n\n";

	/* Get corresponding uuid for filename */
	node.get("/", [&name2file](const std::vector<std::shared_ptr<dht::Value>>& values) {
		for (const auto& value : values) {
			name2file = asciify((*value).data);
		}
		return true;
	});

	sleep(1);
	
	/* With uuid, get all files in directory */
	node.get(name2file, [&fileStringsVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
		for (const auto& value : values) {
			std::string uuid2file = asciify((*value).data);
			fileStringsVec.push_back(uuid2file);
        	}
		return true; // return false to stop the search
	});

	sleep(1);
	
	for (int i = 0; i < fileStringsVec.size(); i++) {
		fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
	}
	
	for (int i = 0; i < fileVec.size(); i++) {
		std::cout << "fileVec[" << i << "] id: " << fileVec[i].id << std::endl;
		std::cout << "fileVec[" << i << "] content: " << fileVec[i].content << std::endl;
		std::cout << "fileVec[" << i << "] isExists: " << fileVec[i].isExists << std::endl;
	}

	std::cout << "\n\nLeaving dht_get...\n\n";
	
}


int main() {
	node.run(4222, dht::crypto::generateIdentity(), true);
        node.bootstrap("bootstrap.jami.net", "4222");

	/* for test purposes */
	dht_init_root();
	dht_mkdir("/A");
	dht_get();	

}


