/* File contains many print statements to show program progress */

/* compile with: g++ newProgram2.cc -std=c++14 -lopendht -lgnutls
   run with:	 ./a.out
*/

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
        ss << std::to_string(data.id) << "##" << std::to_string(data.isExists) << "##" << std::to_string(data.isDirectory) << "##" << data.content;

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
        std::string isDirectory = data.substr(0, data.find("##"));

        data.erase(0, data.find("##") + 2);
        std::string content = data;

        fileObject.id           = std::stoull(id);
        fileObject.isExists     = (isExists == "1");
        fileObject.isDirectory  = (isDirectory == "1");
        fileObject.content      = content;

        return fileObject;
}

/* exclusive put - checks if value already exists in key.
   If it exists, do not put. If it does not exist, put
   Note: value should already be in string form of file
   i.e. id##isExists##isDirectory##content
   this can be done with parseOutgoingDHT) */
bool dht_exclusive_put(std::string key, std::string value) {
        std::vector<std::string> existing_values;

        /* get all existing values */
        node.get(key, [&existing_values](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& value : values) {
                        std::string value_string = asciify((*value).data);
                        existing_values.push_back(value_string);
                }
                return true;
        });

        sleep(1);

        /* extract only the content */
        for (int i = 0; i < existing_values.size(); i++) {
                existing_values[i] = existing_values[i].substr(existing_values[i].find_last_of("##") + 1);
        }

        /* check value against existing values via content */
        std::string value_content = value.substr(value.find_last_of("##") + 1);

        for (int i = 0; i < existing_values.size(); i++) {
                if (value_content == existing_values[i]) {
                        /* return false - did not put value */
                        return false;
                }
        }

        /* At this point, value is new - put it */
        node.put(key, value);
        sleep(1);
        /* return true - did put value */
        return true;
}

/* checks if path_name exists */
bool file_exists(std::string path_name) {
	/* TODO occassionally works - only lightly tested
	 * retest after implementation of rm and compatibility with readdir */
	
	/* checks if uuid exists under path_name 
	 * if there is no corresponding uuid (get returns nothing) - file DNE
	 * if get returns something, check if any of the values are 0
	 	- if value is 0, then has been removed (see rm)
		- if value is not 0, file exists 			*/

	bool uuid_exists = false;
	
	std::cout << "\nChecking if " << path_name << " exists...\n";

	node.get(path_name, [&uuid_exists](const std::vector<std::shared_ptr<dht::Value>>& values) {
                // only reaches this if a value is found
		bool intermediate_check = true;
		for (const auto& value : values) {
                        if (asciify((*value).data) == "0") {
				intermediate_check = false;
			}
                }
		uuid_exists = intermediate_check;
                return true;
        });

	sleep(1);
	return uuid_exists;
}


/* checks if path_name is a directory
   if it is, return true, otherwise false */
bool is_directory(std::string path_name) {
	/* string for converting name to uuid */
        std::string name2file;
        /* string for storing all file strings in uuid */
        std::vector<std::string> fileStringsVec;
        /* vector that contains the conversion of file strings to data type file */
        std::vector<file> fileVec;

        std::cout << "\nChecking if " << path_name << " is a directory...\n\n";
        /* Get corresponding uuid for filename */
        node.get(path_name, [&name2file](const std::vector<std::shared_ptr<dht::Value>>& values) {
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
                return true;
        });

        sleep(1);

	/* TODO - THIS DOES NOT WORK IF path_name DOES NOT EXIST
	 * IT WILL GENERATE RANDOM VALUES AND RETURN YES
	 * FIX WHEN DONE WITH IS_EXISTS (AND RM COMPATIBLE)
	 * */

	std::cout << "the uuid is: " << name2file << std::endl;
        file current_file;
	
	for (int i = 0; i < fileStringsVec.size(); i++) {
                fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
        }
	
        for (int i = 0; i < fileVec.size(); i++) {
		if (fileVec[i].id == stoull(name2file)) {
                        current_file = fileVec[i];
                }
        }
	

	std::cout << "current_file id: " << current_file.id << std::endl;
	std::cout << "current_file isExists: " << current_file.isExists << std::endl;
	std::cout << "current_file isDirectory: " << current_file.isDirectory << std::endl;
	std::cout << "current_file content: " << current_file.content << std::endl;


	if (current_file.isDirectory) {
		return true;
	}
	return false;
}

void dht_mkdir(std::string path_name) {
	unsigned long long file_id = idGenerator();
	std::string parent_uuid;

	std::cout << "Entering dht_mkdir...\n\n";

	/* Put new directory into parent path */
	std::string parent_name = parent_path(path_name);

        //Get uuid (key) of parent
        node.get(parent_name, [&parent_uuid](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& value : values) {
                        parent_uuid = asciify((*value).data);
                }
                return true;
        });

        sleep(1);

	file new_directory(file_id, file_name(path_name), true, true);
	/* Check if parent directory already contains new directory
	   If it does, print directory already exists
	   If it does not, create the new directory */
        if(dht_exclusive_put(parent_uuid, parseOutgoingDHT(new_directory))) {
		std::cout << "Linking new directory to parent...\n";
		sleep(1);

		/* Create new directory */
		std::cout << "Creating new directory on dht...\n";

		new_directory.content = ".";
		/* put <(full)path_name, id> */
		dht_exclusive_put(path_name, std::to_string(file_id));
		/* put <id, <id,.>> */
		dht_exclusive_put(std::to_string(file_id), parseOutgoingDHT(new_directory));
	}
	else {
		std::cout << "mkdir: cannot create directory '" << file_name(path_name) << "': File exists\n";
	}	

	std::cout << "\nLeaving dht_mkdir...\n\n";
}

std::vector<std::string> dht_readdir(std::string path_name) {
	/* string for converting name to uuid */
        std::string name2file;
        /* string for storing all file strings in uuid */
        std::vector<std::string> fileStringsVec;
        /* vector that contains the conversion of file strings to data type file */
        std::vector<file> fileVec;
        /* vector that contains all the file names */
	std::vector<std::string> fileNamesVec;

        std::cout << "Entering dht_readdir...\n\n";
        std::cout << "Getting uuid of '" << path_name << "'...\n";
        /* Get corresponding uuid for filename */
        node.get(path_name, [&name2file](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& value : values) {
                        name2file = asciify((*value).data);
                }
                return true;
        });

        sleep(1);
        std::cout << "Getting contents of '" << name2file << "'...\n";
        /* With uuid, get all files in directory */
        node.get(name2file, [&fileStringsVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& value : values) {
                        std::string uuid2file = asciify((*value).data);
                        fileStringsVec.push_back(uuid2file);
                }
                return true;
        });

        sleep(1);

	file current_file;

        for (int i = 0; i < fileStringsVec.size(); i++) {
                fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
        }

	for (int i = 0; i < fileVec.size(); i++) {
		/*
		std::cout << "fileVec[" << i << "] id: " 	  << fileVec[i].id          << std::endl;
		std::cout << "fileVec[" << i << "] content: " 	  << fileVec[i].content     << std::endl;
		std::cout << "fileVec[" << i << "] isExists: " 	  << fileVec[i].isExists    << std::endl;
		std::cout << "fileVec[" << i << "] isDirectory: " << fileVec[i].isDirectory << std::endl;
		*/
		if (fileVec[i].id == stoull(name2file)) {
			current_file = fileVec[i];
		}
		fileNamesVec.push_back(fileVec[i].content);
		std::cout << std::endl;
	}

	/* Directory check - check if path_name is a directory
           If it is, display all files in directory
           Otherwise, return empty vector and print error */
	if (current_file.isDirectory) {
		std::cout << "Contents of fileNamesVec: \n";
		for (int i = 0; i < fileNamesVec.size(); i++) {
                	std::cout << fileNamesVec[i] << "    ";
        	}
		std::cout << "\nLeaving dht_readdir...\n\n";
        	return fileNamesVec;
	}
	else {
		std::cout << "readdir: cannot read directory '" << file_name(path_name) << "': is not a directory\n";
		fileNamesVec.clear();
		std::cout << "\nLeaving dht_readdir...\n\n";
		return fileNamesVec;
	}	
}

void dht_init_root() {
	std::cout << "initializing root...\n";

        file root_directory(1, ".", true, true);

        //node.put("/", "0");
	dht_exclusive_put("/", "1");
        //node.put("1", parseOutgoingDHT(root_directory));
	dht_exclusive_put("1", parseOutgoingDHT(root_directory));
}

/* remove file at path_name */
void dht_remove(std::string path_name) {
	
}

int main() {
	node.run(4222, dht::crypto::generateIdentity(), true);
        node.bootstrap("bootstrap.jami.net", "4222");

	/* for test purposes */
	dht_init_root();
	dht_mkdir("/A");
	dht_readdir("/");
	
	if(file_exists("/")) {
		std::cout << "file exists\n";	
	}
	else {
		std::cout << "file does not exist\n";
	}
}


