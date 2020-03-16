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
//#include "functions.h"  //contains some dht related functions
//#include "file.h"       //contains definiton of the data structure in dht
//#include "auxiliary_dht_functions.h"
#include "dht_functions.h"

/* Global dhtrunner */
//dht::DhtRunner node;

// To compile this code: g++ p2p.cpp -std=c++14 -o p2p `pkg-config fuse --cflags --libs` -lopendht -lgnutls
// To run this code: ./p2p -f mountpoint

//as defined in fuseDispatcher
using namespace fuse_cpp;

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
        std::string str = "";

       if(!file_exists(s)){
             return -ENOENT;
			 
        }

        else{
                
		//std::string str = "";
                str = dht_read(s);
                strcpy(buf, str.c_str());
        }	
        
       return sizeof(str);       
		//return 0;
}


static int p2pwrite(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
/* convert into string; if file does not exist, throw error. 
Check if file is empty, if empty, get string from buffer and pass it to dht_write
If file is not empty, --        */

        std::string s(path);

        
        if(!file_exists(s)){
                return -ENONET;
        }

        //str = (const char*) dht_read(s);
		//std::string input_string = "";
		//char * cstr = new char [input_string.length() + 1];
		//const char *C = input_string.c_str();
		//strcpy(cstr, input_string.c_str());
		//const char *b = &input_string;
        //strcpy(buf, input_string.data());
	std::string input(buf);
        dht_write(s, input);
       
        return strlen(buf);
}

static int p2ptruncate(const char *path, off_t size) {
	return 0;
}

static int p2prename(const char* from, const char* to) {
        std::string s_from(from);
		std::string s_to(to);
        if(!file_exists(s_from)){
                return -ENONET;
        }

        else{
                dht_rename(s_from, s_to);
        }
        
        return 0;
}

static int p2paccess(const char * path, int mask){
	return 0;
}


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
        dispatcher->set_write	(&p2pwrite);
	dispatcher->set_rmdir	(&p2prmdir);
	dispatcher->set_unlink	(&p2prm);
	dispatcher->set_create	(&p2pcreate);
	dispatcher->set_rename	(&p2prename);
	dispatcher->set_truncate(&p2ptruncate);
	//dispatcher->set_open	(&p2popen);
        dispatcher->set_access	(&p2paccess);

        //call fuse_main() with the operations set above. 
	return fuse_main(argc, argv, (dispatcher->get_fuseOps()), NULL);
}
