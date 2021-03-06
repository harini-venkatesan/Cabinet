/* Data structure for files 
* id: The corresponding unique id of the file, see uuid.cpp
* content: content of the file
        - If file is a directory, content is name of directory
        - If file is a file, content is content of the file
* isExists: determines if file exists. Delete sets isExists = 0
* isDirectory: determines if file is a directory */
class file {
        public:
                unsigned long long id;
                std::string content;
                bool isExists;
                bool isDirectory;

                /* Constructors */
                file();
                file(unsigned long long id, std::string content, bool isExists, bool isDirectory);

                bool operator ==(const file &rhs);

                // implements msgpack-c serialization methods
                // MSGPACK_DEFINE(id, content, isExists);
};

file::file() {

}

file::file(unsigned long long id, std::string content, bool isExists, bool isDirectory) {
        this->id                = id;
        this->content           = content;
        this->isExists          = isExists;
        this->isDirectory       = isDirectory;
}

/* only used for checking if file has been deleted
   so does not check this->isDeleted == rhs.isDeleted */

bool file::operator ==(const file &rhs) {
        return (this->id == rhs.id && this->content == rhs.content && this->isDirectory == rhs.isDirectory);
}
