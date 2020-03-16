/* File contains many print statements to show program progress */
// NEWPROGRAM3 WILL CORRESPOND TO DHT_FUNCTIONS
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
#include "auxiliary_dht_functions.h"

void dht_init_root() {
	std::cout << "initializing root...\n";

	file root_directory(1, ".", true, true);
	dht_exclusive_put("/", "1", 1, 0);
	dht_exclusive_put("1", root_directory.content, 1, 0);
}

void dht_create(std::string path_name) {
	std::cout << "=== dht_create(" << path_name << ") ===\n";
	if (!file_exists(path_name)) {
		unsigned long long file_id = idGenerator();
		std::string parent_uuid;

		/* put new file into parent path */
		parent_uuid = parent_path(path_name);
		parent_uuid = existing_uuid_key(parent_uuid);
		
		if (parent_uuid == "NONE") {
			std::cout << "ERROR3: create: cannot create file '" << path_name << "': No such file or directory\n";
			std::cout << "=== dht_create complete ===\n\n";
			return;
		}

		/* when putting to parent, content is file name, e.g. hello.txt */
		file new_file(file_id, file_name(path_name), true, false);

		if (dht_exclusive_put(parent_uuid, parseOutgoingDHT(new_file), 0, 1)) {
			std::cout << "Linking  '" << path_name << "' to parent...\n";

			/* Create the file on dht */
			std::cout << "Creating '" << path_name << "' on dht with id: " << file_id << std::endl;
			/* put <path, id> */
			dht_exclusive_put(path_name, std::to_string(file_id), 0, 0);
			/* put <id, <id, "", 1, 0>> */
			new_file.content = "";
			dht_exclusive_put(std::to_string(file_id), parseOutgoingDHT(new_file), 0, 1);
		}
		else {
			std::cout << "ERROR2: create: cannot create file '" << path_name << "': File exists\n";
		}
	}
	else {
		std::cout << "ERROR1: create: cannot create file '" << file_name(path_name) << "': File exists\n";
	}
	std::cout << "=== dht_create complete ===\n\n";
}

void dht_mkdir(std::string path_name) {
	std::cout << "=== dht_mkdir(" << path_name << ") ===\n";
	if (!file_exists(path_name)) {
		unsigned long long file_id = idGenerator();
		std::string parent_uuid;
		
		/* put new directory into parent path */
		parent_uuid = parent_path(path_name);
		parent_uuid = existing_uuid_key(parent_uuid);
		
		if (parent_uuid == "NONE") {
			std::cout << "ERROR2: mkdir: cannot create directory '" << path_name << "': No such file or directory\n";
			std::cout << "=== dht_mkdir complete ===\n\n";
			return;
		}

		file new_directory(file_id, file_name(path_name), true, true);

		if (dht_exclusive_put(parent_uuid, parseOutgoingDHT(new_directory), 1, 1)) {
			std::cout << "Linking '" << path_name << "' to parent...\n";
			sleep(1);

			/* Create the directory on dht */
			std::cout << "Creating '" << path_name << "' on dht with id: " << file_id << std::endl;
			/* On directory itself, content is '.' */
			//new_directory.content = ".";
			/* put <path, id> */
			dht_exclusive_put(path_name, std::to_string(file_id), 1, 0);
			/* put <id, <id,., 1, 1>> */
			dht_exclusive_put(std::to_string(file_id), ".", 1, 0);
		}
	}
	else {
		std::cout << "ERROR1: mkdir: cannot create directory '" << path_name << "': Directory exists\n";
	}
	std::cout << "=== dht_mkdir complete ===\n\n";
}

/* remove file at path_name - put the same thing with isExist = 0 */
void dht_remove(std::string path_name) {
	std::cout << "=== dht_remove(" << path_name << ") ===\n";
	if (file_exists(path_name)) {
		/* set isExists = 0 in parent directory */
		std::string path_uuid;
		/* string to hold parent uuid */
		std::string parent_uuid;
		/* vector that stores all file strings */
		std::vector<std::string> fileStringsVec;
		/* vector that stores all files */
		std::vector<file> fileVec;

		if (is_root(path_name)) {
			std::cout << "ERROR2: rm cannot remove '/': directory is the root directory\n";
			return;
		}
		
		path_uuid = existing_uuid_key(path_name);
		parent_uuid = existing_uuid_key(parent_path(path_name));

		/* Get all files in parent */
		node.get(parent_uuid, [&fileStringsVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
				for (const auto& value : values) {
					std::string uuid2file = asciify((*value).data);
					fileStringsVec.push_back(uuid2file);
                               	}
				return true;
		});
		sleep(1);

		for (int i = 0; i < fileStringsVec.size(); i++) {
			fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
		}

		for (int i = 0; i < fileVec.size(); i++) {
			if (fileVec[i].id == stoull(path_uuid)) {
				file remove_file(fileVec[i].id, fileVec[i].content, 0, fileVec[i].isDirectory);
				/* put (labeled) deleted directory to parent */
				node.put(parent_uuid, parseOutgoingDHT(remove_file));
			}
		}
		/* set (add) uuid of path_name to deleted, i.e. uuid + 'D' */
		std::cout << "REMOVING ID: " << path_uuid << std::endl;
		node.put(path_name, path_uuid + 'D');
		sleep(1);
	}
	else {
		std::cout << "ERROR1: rm: cannot remove '" << path_name << "': is not a file or directory\n";
	}
	std::cout << "=== dht_remove complete ===\n\n";
}

/* renames path_from to path_to */
void dht_rename(std::string path_from, std::string path_to) {
	std::cout << "=== dht_rename(" << path_from << ") ===\n";
	if (file_exists(path_from)) {
		if (!file_exists(path_to)) {
			if(is_directory(path_from)) {
				/* TODO renaming a directory */

			}
			else {
				/* renaming a file */
				std::cout << "renaming FILE '" << path_from << "' to '" << path_to << "'...\n";
				std::string path_from_uuid = existing_uuid_key(path_from);
				std::string parent_uuid = existing_uuid_key(parent_path(path_from));
				
				/* file with format to push it to parent */
				file child_file(std::stoull(path_from_uuid), file_name(path_to), 1, 0);
				/* remove <path_from, uuid> entry and <parent_from, <...>> */
				dht_remove(path_from);
				/* put <path_to, uuid of path_from> */
				node.put(path_to, path_from_uuid);
				/* put name of new file in parent */
				node.put(parent_uuid, parseOutgoingDHT(child_file));
				//TODO xcp maybe?
				//dht_exclusive_put(path_to, path_uuid, 0, 0);
				//dht_exclusive_put(parent_uuid, file_name(path_to), 0, 0);
			}
		}
		else {
			/* dst file already exists */
			std::cout << "ERROR2: rename: cannot rename '" << path_to << "': already exists\n";
		}
	}
	else {
		/* src file does not exist */
		std::cout << "ERROR1: rename: cannot rename '" << path_from << "': is not a file or directory\n";
	}
	std::cout << "=== dht_rename complete ===\n\n";
}	

/* return the contents of a file (path_name) */
std::string dht_read(std::string path_name) {
	std::cout << "=== dht_read(" << path_name << ") ===\n";
	if (file_exists(path_name)) {
		if(!is_directory(path_name)) {
			/* string to hold uuid */
			std::string path_uuid;
			/* vector that stores all file strings */
			std::vector<std::string> fileStringsVec;
			/* vector that stores all files */
			std::vector<file> fileVec;
			/* vector that stores all (labeled) deleted files */
			std::vector<file> deletedFilesVec;
                	/* vector that will filter out to only existingFilesVec */
			std::vector<file> filterFilesVec;
			file current_file;

			path_uuid = existing_uuid_key(path_name);
			node.get(path_uuid, [&fileStringsVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
				for (const auto& value : values) {
					std::string uuid2file = asciify((*value).data);
					fileStringsVec.push_back(uuid2file);
				}
				return true;
			});
			sleep(1);

			for (int i = 0; i < fileStringsVec.size(); i++) {
				fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
			}

			for (int i = 0; i < fileVec.size(); i++) {
				/* populate vector of deleted files */
				if (fileVec[i].isExists == false) {
					deletedFilesVec.push_back(fileVec[i]);
				} 
				else {
					filterFilesVec.push_back(fileVec[i]);
				}
			}
			
			/* find the file that has not been deleted */
			for (int i = 0; i < filterFilesVec.size(); i++) {
				bool fileExists = true;
				for (int j = 0; j < deletedFilesVec.size(); j++) {
					if (filterFilesVec[i] == deletedFilesVec[j]) {
						fileExists = false;
					}
				}
				if (fileExists) {
					current_file = filterFilesVec[i];
					break;
				}
			}
			
			std::cout << "=== dht_read complete ===\n\n";
			return current_file.content;
		}
		else {
			std::cout << "ERROR2: read: cannot read file '" << path_name << "'file is a directory\n";
			std::string emptyString;
			std::cout << "=== dht_read complete ===\n\n";
			return emptyString;
	
		}}
	else {
		std::cout << "ERROR1: read: cannot read file '" << path_name << "'file does not exist\n";
		std::string emptyString;
		std::cout << "=== dht_read complete ===\n\n";
		return emptyString;
	}
}

/* append content to a file */
void dht_write(std::string path_name, std::string content) {
	std::cout << "=== dht_write(" << path_name << ") ===\n";
	if (file_exists(path_name)) {
		if (!is_directory(path_name)) {	
			/* string that holds the uuid of the file */
			std::string path_uuid;
			/* file in string format */
			std::string fileString;
			file current_file;
			file delete_file;

			std::cout << "Writing '"<< content << "' to " << file_name(path_name) << "...\n";
			
			//TODO deleted values
			path_uuid = existing_uuid_key(path_name);
			node.get(path_uuid, [&fileString](const std::vector<std::shared_ptr<dht::Value>>& values) {
				for (const auto& value : values) {
					//should only be one value
					fileString = asciify((*value).data);
					}
					return true;
			});
			sleep(1);

			delete_file = parseIncomingDHT(fileString);
			current_file = parseIncomingDHT(fileString);

			/* delete the old file */
			delete_file.isExists = false;
			node.put(path_uuid, parseOutgoingDHT(delete_file));

			/* append the new content */
			current_file.content = current_file.content + content;
			/* put the new file */
			node.put(path_uuid, parseOutgoingDHT(current_file));

			std::cout << "=== dht_write complete ===\n\n";
		}
		else {
			std::cout << "ERROR2: write: cannot write to '" << path_name << "file is a directory\n";
		}
	}
	else {
		std::cout << "ERROR1: write: cannot write to '" << path_name << "file does not exist\n";
	}
}

std::vector<std::string> dht_readdir(std::string path_name) {
	std::cout << "=== dht_readdir(" << path_name << ")===\n";
	if (is_directory(path_name)) {
		/* string that holds the uuid of the file */
		std::string fileUuid;
		/* vector that stores all file strings */
		std::vector<std::string> fileStringsVec;
		/* vector that stores all files */
		std::vector<file> fileVec;
		/* vector that stores all (labeled) deleted files */
		std::vector<file> deletedFilesVec;
		/* vector that will filter out to only existingFilesVec */
		std::vector<file> filterFilesVec;
		/* vector that contains the existing files */
		std::vector<file> existingFilesVec;
		/* vector that contains names of all files */
		std::vector<std::string> fileNamesVec;
		
		fileUuid = existing_uuid_key(path_name);

		/* With uuid, get all files in directory */
		node.get(fileUuid, [&fileStringsVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
			for (const auto& value : values) {
				std::string uuid2file = asciify((*value).data);
				fileStringsVec.push_back(uuid2file);
			}
			return true;
		});
		sleep(1);

		for (int i = 0; i < fileStringsVec.size(); i++) {
			fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
		}

		for (int i = 0; i < fileVec.size(); i++) {
			/* populate vector of deleted files */
			if (fileVec[i].isExists == false) {
				deletedFilesVec.push_back(fileVec[i]);
			}
			else {
				filterFilesVec.push_back(fileVec[i]);
			}
		}
		
		/* remove files that have been (labeled as) deleted */
		for (int i = 0; i < filterFilesVec.size(); i++) {
			bool fileExists = true;
			for (int j = 0; j < deletedFilesVec.size(); j++)  {
				if (filterFilesVec[i] == deletedFilesVec[j]) {
					fileExists = false;
				}
			}
			if (fileExists) {
				existingFilesVec.push_back(filterFilesVec[i]);
			}
		}

		/* extract file names */
		for (int i = 0; i < existingFilesVec.size(); i++) {
			fileNamesVec.push_back(existingFilesVec[i].content);
		}

		/* print file names */
		std::cout << "CONTENTS OF " << path_name << ":\n";
		for (int i = 0; i < fileNamesVec.size(); i++) {
			std::cout << fileNamesVec[i] << "     ";
		}
		std::cout << std::endl;
		std::cout << "=== dht_readdir complete ===\n\n";
		return fileNamesVec;
	}
	else {
		std::cout << "ERROR1: readdir: cannot readdir '" << path_name << "': is not a directory\n";
		
		std::vector<std::string> emptyVec;
		std::cout << "=== dht_readdir complete ===\n\n";
		return emptyVec;
	}
}

int main() {
	node.run(4222, dht::crypto::generateIdentity(), true);
        node.bootstrap("bootstrap.jami.net", "4222");
	
	/* for test purposes */


	/* testing rename on a FILE... */
	dht_init_root();
	dht_mkdir("/A");
	dht_create("/A/hello.txt");
	dht_readdir("/A");
	dht_rename("/A/hello.txt", "/A/world.txt");
	dht_readdir("/A");

	/* //testing read and write... 
	dht_init_root();
	dht_mkdir("/A");
	dht_create("/A/hello.txt");
	std::string AString = dht_read("/A/hello.txt");
	std::cout << "Astring: " << AString << std::endl;
	dht_write("/A/hello.txt", " how are you?");
	std::string BString = dht_read("/A/hello.txt");
	std::cout << "Bstring: " << BString << std::endl;
	*/

	/* //testing remove... 
	dht_init_root();
	dht_mkdir("/Z");
	dht_readdir("/");
	dht_remove("/Z");
	dht_create("/Z/hello.txt");
	dht_readdir("/");
	dht_mkdir("/Z");
	dht_create("/Z/world.txt");
	dht_readdir("/");
	dht_readdir("/Z");
	*/
	
	/* //general testing...
	dht_mkdir("/A");
	dht_mkdir("/B");
	dht_readdir("/");
	dht_remove("/A");
	dht_readdir("/");
	dht_readdir("/A");
	dht_readdir("/B");
	dht_mkdir("/A");
	dht_readdir("/");
	dht_mkdir("/ZXCV/VBN");
	dht_create("/QWER/ASDF.txt");
	dht_create("/A/hello.txt");
	dht_readdir("/A");
	*/
}

