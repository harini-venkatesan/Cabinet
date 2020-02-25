/* Method to enforce a pseudo-UUID system. Uses a 64-bit integer instead. */

/* Create a unique number for each file */
unsigned long long idGenerator() {
	std::random_device rd;
	std::default_random_engine generator(rd());
	/* 1 is reserved for the root */
	std::uniform_int_distribution<long long unsigned> distribution(2,0xFFFFFFFFFFFFFFFF);

	return distribution(generator);
}

int main() {
	/* For test purposes */
	unsigned long long id = idGenerator();
	printf("Randomly generated number: %llu\n", id);

	return 0;
}
