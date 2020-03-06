/* Create a unique number for each file */
unsigned long long idGenerator() {
        std::random_device rd;
        std::default_random_engine generator(rd());
        /* 1 is reserved for the root */
        std::uniform_int_distribution<long long unsigned> distribution(2,0xFFFFFFFFFFFFFFFF);

        return distribution(generator);
}

/* Find parent path given a path */
std::string parent_path(std::string path_name) {
        std::string parent_path = path_name.substr(0, path_name.find_last_of("/"));

        /* If empty, parent is the root */
        if (parent_path == "") {
                parent_path = "/";
        }
        return parent_path;
}

/* Find file name given full path */
std::string file_name(std::string path_name) {
        std::string file_name = path_name.substr(path_name.find_last_of("/") + 1);

        return file_name;
}

/* Converts vector to ASCII string */
std::string asciify(std::vector<auto> const &vec) {
    std::string fullString;

    for (int i = 1; i < vec.size(); i++) {
        fullString += static_cast<char>(vec[i]);
    }

    return fullString;
}

/* Checks if path_name is root */
bool is_root(std::string path_name) {
        if (path_name == "/") {
                return true;
        }
        return false;
}
