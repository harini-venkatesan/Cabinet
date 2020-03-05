/* Data structure for files 
* id: The corresponding unique id of the file, see uuid.cpp
* content: content of the file
	- If file is a directory, content is name of directory
	- If file is a file, content is content of the file
* isExists: determines if file exists. Delete sets isExists = 0 */
class file {
        public:
                unsigned long long id;
                std::string content;
                bool isExists;

                /* Constructors */
                file();
                file(unsigned long long id, std::string content, bool isExists);
                // implements msgpack-c serialization methods
                // MSGPACK_DEFINE(id, content, isExists);
};

file::file() {

}

file::file(unsigned long long id, std::string content, bool isExists) {
        this->id 	= id;
        this->content 	= content;
        this->isExists 	= isExists;
}
