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
   this can be done with parseOutgoingDHT) 
   TODO: make xcp more robust
   TODO: DOES NOT WORK WITH <PATH, UUID> CHECK
   */
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

        sleep(2);

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
	if (file_exists(path_name)) {
		/* string for converting name to uuid */
        	std::string name2file;
        	/* vector for storing all file strings in uuid */
        	std::vector<std::string> fileStringsVec;
        	/* vector that contains the conversion of file strings to data type file */
        	std::vector<file> fileVec;
        	/* vector that contains all the file names */
        	std::vector<std::string> fileNamesVec;
		/* vector that contains the deleted files */
		std::vector<file> deletedFilesVec;
		/* vector that contains the existing files */
		std::vector<file> existingFilesVec;

        	std::cout << "Entering dht_readdir...\n\n";

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

        	file current_file;

        	for (int i = 0; i < fileStringsVec.size(); i++) {
                	fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
        	}

		for (int i = 0; i < fileVec.size(); i++) {
			/*
                	std::cout << "fileVec[" << i << "] id: "          << fileVec[i].id          << std::endl;
                	std::cout << "fileVec[" << i << "] content: "     << fileVec[i].content     << std::endl;
                	std::cout << "fileVec[" << i << "] isExists: "    << fileVec[i].isExists    << std::endl;
                	std::cout << "fileVec[" << i << "] isDirectory: " << fileVec[i].isDirectory << std::endl;
			std::cout << std::endl;
			*/
			/* Construct vector of deleted files */
			if (fileVec[i].isExists == false) {
				deletedFilesVec.push_back(fileVec[i]);
			}
                	if (fileVec[i].id == stoull(name2file)) {
                        	current_file = fileVec[i];
                	}
        	}
		
		for (int i = 0; i < fileVec.size(); i++) {
			bool fileExists = true;
			for (int j = 0; j < deletedFilesVec.size(); j++) {
				if (fileVec[i] == deletedFilesVec[j]) {
					std::cout << "THIS FILE HAS BEEN DELETED\n";
					fileExists = false;
				}
			}
			if (fileExists) {
				existingFilesVec.push_back(fileVec[i]);
			}
		}

		for (int i = 0; i < existingFilesVec.size(); i++) {
			fileNamesVec.push_back(existingFilesVec[i].content);
		}

        	/* Directory check - check if path_name is a directory
           	If it is, display all files in directory
           	Otherwise, return empty vector and print error */
        	if (current_file.isDirectory) {
                	std::cout << "Contents of " << path_name << ":\n";
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
	else {
		std::cout << "readdir: cannot readdir '" << path_name << "': No such file or directory\n";
		std::vector<std::string> emptyVec;
		return emptyVec;
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

/* remove file at path_name 
   NOTE: not exclusive put, uses regular put
   puts same thing with isExist = 0
*/

void dht_remove(std::string path_name) {
	if (file_exists(path_name)) {
		/* set the isExists in the parent of path_name to 0 (with uuid) */
		
		/* string for converting name to uuid */
        	std::string path_uuid;
        	/* string to hold parent uuid */
		std::string parent_uuid;	
		/* vector for storing all file strings in parent uuid */
        	std::vector<std::string> fileStringsVec;
		/* vector that contains the conversion of file strings to data type file */
        	std::vector<file> fileVec;
       	
		std::cout << "Removing " << path_name << " ...\n";

		/* Check if path_name is root - (cannot remove root)
		   	- If is root, throw an error
			- Otherwise, continue 			*/
		if (path_name == "/") {
			std::cout << "rm cannot remove '/': directory is the root directory\n";
			return;
		}

		/* Get corresponding uuid for filename */
        	node.get(path_name, [&path_uuid](const std::vector<std::shared_ptr<dht::Value>>& values) {
                	for (const auto& value : values) {
                        	if (asciify((*value).data) != "0") {
					path_uuid = asciify((*value).data);
				}
			//name2file = asciify((*value).data);
                	}
                	return true;
        	});

		sleep(2);

		/* Get corresponding uuid for parent */
                node.get(parent_path(path_name), [&parent_uuid](const std::vector<std::shared_ptr<dht::Value>>& values) {
                	for (const auto& value : values) {
				parent_uuid = asciify((*value).data);
                	}
                	return true;
                });

                sleep(2);

		std::cout << "Getting contents of parent_uuid...\n";
		/* With uuid, get all files in directory */
		node.get(parent_uuid, [&fileStringsVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
                	for (const auto& value : values) {
                        	std::string uuid2file = asciify((*value).data);
                        	fileStringsVec.push_back(uuid2file);
                	}
                	return true;
        	});

        	sleep(2);

		for (int i = 0; i < fileStringsVec.size(); i++) {
                	fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
		}

		for (int i = 0; i < fileVec.size(); i++) {
			if (fileVec[i].id == stoull(path_uuid)) {
				std::cout << "FOUND THE CORRECT UUID\n";
				file remove_file(fileVec[i].id, fileVec[i].content, 0, fileVec[i].isDirectory);
				/* put labeled removed directory to parent */
                		node.put(parent_uuid, parseOutgoingDHT(remove_file));
			}
		}
		/* set (add) uuid of path_name to 0 */
		node.put(path_name, "0");
	}
	else {
		std::cout << "rm: cannot remove '" << path_name << "': No such file or directory\n";
	}
}

void tempGet(std::string path_name) {
	std::cout << "Entering tempGet with " << path_name << " ...\n";

	node.get(path_name, [](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& value : values) {
			std::cout << asciify((*value).data) << std::endl;
                }
                return true;
        });

	sleep(2);

}

int main() {
	node.run(4222, dht::crypto::generateIdentity(), true);
        node.bootstrap("bootstrap.jami.net", "4222");

	/* for test purposes */
	dht_init_root();
	dht_mkdir("/A");
	dht_mkdir("/B");
	//dht_readdir("/A");
	
	std::cout << "/B should appear here \n";
	dht_readdir("/");

	dht_remove("/C");
	dht_remove("/B");

	//tempGet("1");
	//tempGet("/B");
	std::cout << "/B should no longer appear here\n";
	dht_readdir("/");

	if(file_exists("/A")) {
		std::cout << "file exists\n";	
	}
	else {
		std::cout << "file does not exist\n";
	}
}

