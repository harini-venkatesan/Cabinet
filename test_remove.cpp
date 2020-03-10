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
#include "auxiliary_dht_functions.h"

/* Global dhtrunner */
//dht::DhtRunner node;

// To compile this code: g++ p2p.cpp -std=c++14 -o p2p `pkg-config fuse --cflags --libs` -lopendht -lgnutls
// To run this code: ./p2p -f mountpoint

//as defined in fuseDispatcher
using namespace fuse_cpp;

void dht_init_root() {
	std::cout << "initializing root...\n";

	file root_directory(1, ".", true, true);
	dht_exclusive_put("/", "1", 1, 0);
	dht_exclusive_put("1", root_directory.content, 1, 0);
}

void dht_create(std::string path_name) {
	if (!file_exists(path_name)) {
		unsigned long long file_id = idGenerator();
		std::string parent_uuid;

		std::cout << "=== dht_create(" << path_name << ") ===\n";

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

/* remove file at path_name 
   idea: put the same thing with isExist = 0 */
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

/* return the contents of a file (path_name) */
std::string dht_read(std::string path_name) {
	std::cout << "=== dht_read(" << path_name << ") ===\n";
	if (file_exists(path_name)) {
		if(!is_directory(path_name)) {
			/* string to hold uuid */
			std::string path_uuid;
			/* file in string format */
			std::string fileString;
			file current_file;

			path_uuid = existing_uuid_key(path_name);
			node.get(path_uuid, [&fileString](const std::vector<std::shared_ptr<dht::Value>>& values) {
				for (const auto& value : values) {
					//should only be one value
					fileString = asciify((*value).data);
				}
				return true;
			});
			sleep(1);

			current_file = parseIncomingDHT(fileString);
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
		if (file_exists(path_name)) {
			std::cout << "AT LEAST I PASSED FILE EXISTS\n";
		}
		else {
			std::cout << "I FAILED FILE EXISTS TOO\n";
		}
		std::vector<std::string> emptyVec;
		std::cout << "=== dht_readdir complete ===\n\n";
		return emptyVec;
	}
}



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

    
  else if(!file_exists(s)){
	  return -ENOENT;
  }

  else if (file_exists(s)) {
    stbuf->st_mode = S_IFREG | 0777;
    stbuf->st_nlink = 1;
    //std::string str = dht_read(s);  
    //stbuf->st_size = 3;
	return 0;
    }

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


static int p2pcreate(const char * path, mode_t mode, struct fuse_file_info *fi) {

        std::string s(path);
		mode = S_IFREG | 0777;
        dht_create(s);

		if(!file_exists(s)){
			return -ENOENT;
		}

		return 0;
}


static int p2pread(const char *path, char *buf, size_t size, off_t offset,struct fuse_file_info *fi) {
/* convert to string. If the file does not exist, call error. Else  */
        std::string s(path);

       if(!file_exists(s)){
             return -ENOENT;
			 
        }

        else{
                
				std::string str = "";
                str = dht_read(s);
                strcpy(buf, str.c_str());
        }	
        
       return sizeof(buf);       
		//return 0;
}

/*
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

        node.run(4395, dht::crypto::generateIdentity(), true);
        //node.bootstrap("bootstrap.jami.net", "4222");

	dispatcher = new FuseDispatcher();
	
	dispatcher->set_mkdir   (&p2pmkdir);
	dispatcher->set_getattr	(&p2pgetattr);
	dispatcher->set_readdir	(&p2preaddir);
    dispatcher->set_init    (&p2pinit);
	dispatcher->set_read	(&p2pread);
        //dispatcher->set_write	(&p2pwrite);
	dispatcher->set_rmdir	(&p2prmdir);
	dispatcher->set_unlink	(&p2prm);
	dispatcher->set_create	(&p2pcreate);
	//dispatcher->set_rename	(&p2prename);
	//dispatcher->set_truncate(&p2ptruncate);
	//dispatcher->set_open	(&p2popen);
        //dispatcher->set_access	(&p2paccess);

        //call fuse_main() with the operations set above. 
	return fuse_main(argc, argv, (dispatcher->get_fuseOps()), NULL);
}