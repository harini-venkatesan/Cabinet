#define FUSE_USE_VERSION 30

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <random>
#include <opendht.h>
#include <sstream>

#include "fusecpp.h"    //contains header for c to c+ conversion of fuse
#include "functions.h"  //contains some dht related functions
#include "file.h"       //contains definiton of the data structure in dht

/* Global dhtrunner */
dht::DhtRunner node;

// To compile this code: g++ p2p.cpp -std=c++14 -o p2p `pkg-config fuse --cflags --libs` -lopendht -lgnutls
// To run this code: ./p2p -f mountpoint

//as defined in fuseDispatcher
using namespace fuse_cpp;


//////////////////////////////////// openDHT functions below //////////////////////////////////

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
	
	std::cout << "file_exists(" << path_name << ")...\n";

	node.get(path_name, [&uuid_exists, &path_name](const std::vector<std::shared_ptr<dht::Value>>& values) {
                // only reaches this if a value is found
		bool intermediate_check = true;
		for (const auto& value : values) {
			std::cout << "the value i got for " << path_name << " is: " << asciify((*value).data) << std::endl;
                        if (asciify((*value).data) == "0") {
				intermediate_check = false;
			}
                }
		uuid_exists = intermediate_check;
                return true;
    });
    std::cout << "uuid_exist value is: " << uuid_exists << std::endl;
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
        if (name2file.empty()) {
            return false;
        }
	

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




//////////////////////////// fuse functions below //////////////////////////////////////////


static void* p2pinit (struct fuse_conn_info *conn) {
/* calls dht init root to initialize mountpoint as root '/' */
        dht_init_root();

}

static int p2pgetattr(const char *path, struct stat *stbuf) {
/*if the value of path equals to root /, we declare it as a directory and return.
if the value of path equals to filepath /file, we declare it as a file and explicit its size and then return.
Otherwise nothing exists at the given path, and we return -ENOENT.
*/
    std::string s(path);
    if (is_root(s) || is_directory(s)) {
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    return 0;
    }

    

  if (file_exists(s)) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    stbuf->st_size = 100;  //IMPORTANT TODO: create a file that returns content, find the size of the content
    return 0;
    }

  return -ENOENT;

}

static int p2preaddir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
/* Like a standard readdir function, fill buffer for common cases, then check if not a directory.
If not a directory, call error. Else, conver into string, return a vector of strings from dht_readdir.
loop within the vector of strings to pass into the buffer for string by string.  */	

        filler(buffer, ".", NULL, 0 );
	filler(buffer, "..", NULL, 0 );
        const char * ls_string;
        std::string s(path);

    if(!is_directory(s))
        return -ENOENT;

    else{
        std::string s(path); 
        std::vector<std::string> ls_vector;
        //in a for loop with key, value pair -> print the files in a string
        ls_vector = dht_readdir(s);

        for(int i=0; i < ls_vector.size(); i++){
            ls_string = ls_vector[i].c_str();
            filler(buffer, ls_string, NULL, 0);
        }
        
    }

    return 0;
}


static int p2pmkdir(const char *path, mode_t mode) {
/* convert const char* to string for dht_mkdir() purposes; calls dht_mkdir() */

    std::string s(path); 
    dht_mkdir(s);

    return 0;

}

static int p2prmdir(const char * path){
/* conver const char* to string. check if not a directory, return error. Check is the file does not exist, return error
else call dht_rmdir function */
        std::string s(path); 

        if(!is_directory(s))
            return -ENOENT;
        if(!file_exists(s)){
            std::cout << "fuse function here" << std::endl;
            return -ENOENT;
        }
        else{    
            dht_remove(s);
        }
    
    return 0;

}

static int p2prm(const char * path){
/* conver const char* to string. check if it is a directory, return error. Check is the file does not exist, return error
else call dht_rm function */
        std::string s(path); 

        if(is_directory(s))
            return -ENOENT;
        if(!file_exists(s))
            return -ENOENT;
        else{    
             dht_remove(s);
        }
    
    return 0;

}

/*
static int p2pcreate(const char * path, mode_t mode, struct fuse_file_info *fi) {

        std::string s(pa th);

        dht_create(s);
}

static int p2pread(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi) {
/* convert to string. If the file does not exist, call error. Else  
        std::string s(path);

        if(!file_exists(s)){
             return -ENOENT;
        }

        else{
                const char * str = "";
                str = (const char *)dht_read(s);
                strcpy(buf, str)
        }
        
        return  size(str);       

}

static int mywrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
/* convert into string; if file does not exist, throw error. 
Check if file is empty, if empty, get string from buffer and pass it to dht_write
If file is not empty, --        
        std::string s(path);

        
        if(!file_exists(s)){
                return -ENONET;
        }

        str = (const char*) dht_read(s);

        //if empty or not empty, can he either append to null or existing? 
        if(size(str) == 0){
                std::string input_string = "";
                strcpy(buf, input_string);
                dht_write(input_string);
        }

        if(size(str > 0)){
                std::string input_string = "";
                strcpy(buf,input_string);
                dht_write(input_string);
        }

        return strlen(buf);
}

static int myrename(const char* from, const char* to) {
        std::string s_from(from);

        if(!file_exists(s_from)){
                return -ENONET;
        }

        else{
                dht_rename(s_from, s_to);
        }
        
        return 0;
}
*/

int main( int argc, char *argv[] ) {
        //FuseDispatcher is a c++ binding of Fuse to make it be compatible with openDHT which runs with c++
        // Reference: http://www.circlesoft.com/fusecpp.h
	FuseDispatcher *dispatcher;

        node.run(4389, dht::crypto::generateIdentity(), true);
        //node.bootstrap("bootstrap.jami.net", "4222");

	dispatcher = new FuseDispatcher();
	
	dispatcher->set_mkdir   (&p2pmkdir);
	dispatcher->set_getattr	(&p2pgetattr);
	dispatcher->set_readdir	(&p2preaddir);
        dispatcher->set_init    (&p2pinit);
	//dispatcher->set_read	(&p2pread);
        //dispatcher->set_write	(&p2pwrite);
	dispatcher->set_rmdir	(&p2prmdir);
	dispatcher->set_unlink	(&p2prm);
	//dispatcher->set_create	(&p2pcreate);
	//dispatcher->set_rename	(&p2prename);
	//dispatcher->set_truncate(&p2ptruncate);
	//dispatcher->set_open	(&p2popen);
        //dispatcher->set_access	(&p2paccess);

        //call fuse_main() with the operations set above. 
	return fuse_main(argc, argv, (dispatcher->get_fuseOps()), NULL);
}
