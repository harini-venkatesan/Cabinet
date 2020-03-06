#define FUSE_USE_VERSION 30

//#include <fuse.h>
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

#include "fusecpp.h"
#include "functions.h"
#include "file.h"

/* Global dhtrunner */
dht::DhtRunner node;

// gcc FS.c -o FS `pkg-config fuse --cflags --libs`
// ./ FS - f Desktop / OS / mountpoint4
using namespace fuse_cpp;

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

std::vector<std::string> dht_readdir(std::string path_name) {
        /* string for converting name to uuid */
        std::string name2file;
        /* string for storing all file strings in uuid */
        std::vector<std::string> fileStringsVec;
        /* vector that contains the conversion of file strings to data type file */
        std::vector<file> fileVec;
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

        for (int i = 0; i < fileStringsVec.size(); i++) {
                fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
        }

        for (int i = 0; i < fileVec.size(); i++) {
                /*
                std::cout << "fileVec[" << i << "] id: " << fileVec[i].id << std::endl;
                std::cout << "fileVec[" << i << "] content: " << fileVec[i].content << std::endl;
                std::cout << "fileVec[" << i << "] isExists: " << fileVec[i].isExists << std::endl;
                */
                fileNamesVec.push_back(fileVec[i].content);
                std::cout << std::endl;
        }

        std::cout << "Contents of fileNamesVec: \n";
        for (int i = 0; i < fileNamesVec.size(); i++) {
                std::cout << fileNamesVec[i] << "    ";
        }

        std::cout << "\nLeaving dht_readdir...\n\n";
        return fileNamesVec;
}

static int p2pmkdir(const char *path, mode_t mode) {

    std::string s(path); 
    dht_mkdir(s);

    return 0;

}

static int p2pgetattr(const char *path, struct stat *statit) {
    /*if the value of path equals to root /, we declare it as a directory and return.
if the value of path equals to filepath /file, we declare it as a file and explicit its size and then return.
Otherwise nothing exists at the given path, and we return -ENOENT.
*/

    if (path = '/') {
    statit->st_mode = S_IFDIR | 0755;
    statit->st_nlink = 2;
    return 0;
    }

  if (strcmp(path, filepath) == 0) {
    statit->st_mode = S_IFREG | 0777;
    statit->st_nlink = 1;
    statit->st_size = strlen(length of file);
    return 0;
    }

  return -ENOENT;

}

int p2preaddir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi ){
	
    //check to see if it is a directory on dht

	filler(buffer, ".", NULL, 0 );
	filler(buffer, "..", NULL, 0 );
    std::string ls_string;

    if(not_a_dir)
        return -ENOENT;

    else{
        std::string s(path); 
        std::vector<std::string> ls_vector;
        //in a for loop with key, value pair -> print the files in a string
        ls_vector = dht_readdir(s);

        for(int i=0; i < ls_vector.size(); i++){
            filler(buffer, (const char*)ls_string[i], NULL, 0);
        }
        
    }

    return 0;
}


int p2prmdir(const char * path){
        std::string s(path); 

        if(not_dir)
            return -ENOENT;
        if(doesnt_exist)
            return -ENOENT;
        else{    
        dht_rmdir(s);
        }
    
    return 0;

}

int p2prm(const char * path){
        std::string s(path); 

        if(dir)
            return -ENOENT;
        if(doesnt_exist)
            return -ENOENT;
        else{    
        dht_rm(s);
        }
    
    return 0;

}



int main( int argc, char *argv[] ) {
	FuseDispatcher *dispatcher;

    node.run(4222, dht::crypto::generateIdentity(), true);
    node.bootstrap("bootstrap.jami.net", "4222");

	/* for test purposes */
	//dht_init_root();
	//dht_mkdir("/A");
	//dht_readdir("/");
    
    //initialize dht nodes and functions here 
	dispatcher = new FuseDispatcher();
	
	dispatcher->set_mkdir   (&p2pmkdir);
	dispatcher->set_getattr	(&p2pgetattr);
	dispatcher->set_readdir	(&p2preaddir);
	//dispatcher->set_open	(&p2popen);
	//dispatcher->set_read	(&p2pread);
	dispatcher->set_rmdir	(&p2prmdir);
	dispatcher->set_unlink	(&p2prm);
	//dispatcher->set_create	(&p2pcreate);
	//dispatcher->set_access	(&p2paccess);
	//dispatcher->set_rename	(&p2prename);
	//dispatcher->set_truncate(&p2ptruncate);
	//dispatcher->set_write	(&p2pwrite);


	return fuse_main(argc, argv, (dispatcher->get_fuseOps()), NULL);
}
