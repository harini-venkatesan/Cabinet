/* Method to enforce a pseudo-UUID system. Uses a 64-bit integer instead. */

class idContentPair {
	public:
		unsigned long long id;
		std::string content;
		
		/* Constructor */
		idContentPair(unsigned long long id, std::string content);
		
		/* implements msgpack-c serialization methods */
		MSGPACK_DEFINE(id, content);
};

/* Constructor */
idContentPair::idContentPair(unsigned long long id, std::string content) {
	this->id = id;
	this->content = content;
}

/* Create a unique number for each file */
unsigned long long idGenerator() {
	std::random_device rd;
	std::default_random_engine generator(rd());
	/* 1 is reserved for the root */
	std::uniform_int_distribution<long long unsigned> distribution(2,0xFFFFFFFFFFFFFFFF);

	return distribution(generator);
}

/* Create root directory '/' */
void initRoot() {
        /* Root will always have id 1 */
		// TODO
		// bootstrap.put("1", <., 1>)
		// bootstrap.put("1", <.., DNE>)
}

int main() {
	/* For test purposes */
	unsigned long long id = idGenerator();
	printf("Randomly generated number: %llu\n", id);

	return 0;
}
