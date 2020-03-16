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

/* finds non-deleted uuid for <path, uuid> pairs
   see wiki on remove for more details  */
std::string existing_uuid_key(std::string path_name) {
	/* vector containing all uuid strings in path */
	std::vector<std::string> uuidVec;
	/* Contains all uuids that have been labeled as deleted */
	std::vector<std::string> deletedUuidsVec;
	/* Contains all uuids do not have deleted label, but may have been */
	std::vector<std::string> existingUuidsVec;
	/* existingKey should be a non-deleted uuid if one exists */
	std::string existingKey = "NONE";

	/* populate uuidVec */
	node.get(path_name, [&uuidVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
		for (const auto& value : values) {
			std::string uuidString = asciify((*value).data);
			uuidVec.push_back(uuidString);
		}
		return true;
	});
	sleep(2);

	/* sort uuids
		- if marked with 'D' at the end, it has been deleted
		- otherwise, it can potentially exist   */
	for (int i = 0; i < uuidVec.size(); i++) {
		if (uuidVec[i].back() == 'D') {
			deletedUuidsVec.push_back(uuidVec[i]);
		}
		else {
			existingUuidsVec.push_back(uuidVec[i]);
		}
	}

	for (int i = 0; i < deletedUuidsVec.size(); i++) {
		deletedUuidsVec[i].pop_back();
	}

	/* check existing uuids against deleted uuids */
	for (int i = 0; i < existingUuidsVec.size(); i++) {
		//there should at most one
		bool isExists = true;
		for (int j = 0; j < deletedUuidsVec.size(); j++) {
			if (existingUuidsVec[i] == deletedUuidsVec[j]) {
				isExists = false;
			}
		}
		if (isExists) {
			existingKey = existingUuidsVec[i];
		}
	}

	return existingKey;
}

/* checks if path_name exists
   checks at <path, uuid> level */
bool file_exists(std::string path_name) {
	/* vector containing all uuid strings in path */
	std::vector<std::string> uuidVec;
	/* Contains all uuids that have been labeled as deleted */
        std::vector<std::string> deletedUuidsVec;
	/* Contains all uuids do not have deleted label, but may have been */
	std::vector<std::string> existingUuidsVec;
	/* existingKey should be a non-deleted uuid if one exists */
	std::string existingKey = "NONE";

	/* populate uuidVec */
	node.get(path_name, [&uuidVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
		for (const auto& value : values) {
			std::string uuidString = asciify((*value).data);
			uuidVec.push_back(uuidString);
		}
		return true;
	});
        sleep(2);

        /* sort uuids
	 	- if marked with 'D' at the end, it has been deleted
	        - otherwise, it can potentially exist	*/
        for (int i = 0; i < uuidVec.size(); i++) {
                if (uuidVec[i].back() == 'D') {
                        deletedUuidsVec.push_back(uuidVec[i]);
                }
		else {
			existingUuidsVec.push_back(uuidVec[i]);
		}
        }

	for (int i = 0; i < deletedUuidsVec.size(); i++) {
		deletedUuidsVec[i].pop_back();
	}

	/* check existing uuids against deleted uuids */
	for (int i = 0; i < existingUuidsVec.size(); i++) {
		//there should at most one
		bool isExists = true;
		for (int j = 0; j < deletedUuidsVec.size(); j++) {
			if (existingUuidsVec[i] == deletedUuidsVec[j]) {
				isExists = false;
			}
		}
		if (isExists) {
			existingKey = existingUuidsVec[i];
		}
	}

	if (existingKey == "NONE") {
		return false;
	}
	return true;
}

/* checks if path_name is a directory
   if it is, return true, otherwise false */
bool is_directory(std::string path_name) {
	/* Check if the file exists */
	if (file_exists(path_name)) {
		/* string for converting name to uuid */
		std::string path_uuid = existing_uuid_key(path_name);
		/* vector for storing all file strings in uuid */
        	std::vector<std::string> fileStringsVec;
		/* vector that contains the conversion of file strings to data type file */
		std::vector<file> fileVec;
		/* get current file based on id */
		file current_file;

		/* populate fileStringsVec based on uuid */
		node.get(path_uuid, [&fileStringsVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
			for (const auto& value : values) {
				std::string uuidString = asciify((*value).data);
				fileStringsVec.push_back(uuidString);
			}
			return true;
		});
        	sleep(1);

		for (int i = 0; i < fileStringsVec.size(); i++) {
			fileVec.push_back(parseIncomingDHT(fileStringsVec[i]));
		}

		for (int i = 0; i < fileVec.size(); i++) {
			if (std::stoull(path_uuid) == fileVec[i].id) {
				current_file = fileVec[i];
			}
		}

		if (current_file.isDirectory) {
			return true;
		}
		return false;
	}
	else {
		return false;
	}
}



/* exclusive put - checks if value already exists in key.
   	- If it exists, do not put 
	- If it does not exist, put
	- verbatim parameter tell to directly put the value without parsing
		(generally used for putting at a different key than self)
   TODO: never need to use xcp with <path, uuid> pairs
   intended use is for <uuid, <uuid, content, E, D>> pairs
   */
bool dht_exclusive_put(std::string key, std::string value, bool isDirectory, bool verbatim) {
	bool keyIsUuid = true;
	try {
		std::stoull(key);
	}
	catch (std::invalid_argument) {
		/* key is a path instead => <path, uuid> pair */
		keyIsUuid = false;
	}

	if (keyIsUuid) {
		std::vector<std::string> existingValuesVec;
		std::vector<file> fileVec;
		bool toPut = true;

		/* get all existing values at key */
		node.get(key, [&existingValuesVec](const std::vector<std::shared_ptr<dht::Value>>& values) {
			for (const auto& value : values) {
				std::string value_string = asciify((*value).data);
				existingValuesVec.push_back(value_string);
			}
			return true;
		});

		sleep(1);

		/* extract into file objects */
		for (int i = 0; i < existingValuesVec.size(); i++) {
			fileVec.push_back(parseIncomingDHT(existingValuesVec[i]));
		}
		
		/* check if content matches */
		for(int i = 0; i < fileVec.size(); i++) {
			if (value == fileVec[i].content) {
				toPut = false;
			}
		}

		if (toPut) {
			if (verbatim) {
				node.put(key, value);
				sleep(1);
				return true;
			}
			file new_file(stoull(key), value, 1, isDirectory);
			node.put(key, parseOutgoingDHT(new_file));
			sleep(1);
			return true;
		}
		return false;
	}
	else { 	
		// key is a path, check if uuid exists in path
		// if one exists, don't put
		// if does not exist, put <key, value>
		if (file_exists(key)) {
			return false;
		}
		node.put(key, value);
		return true;
	}
}

