/*
   Compile with: g++ test.cc -std=c++14 -lopendht -lgnutls
   Run with    : ./a.out
*/

#include "dht_functions.h"
#include <chrono>

int main() {
	node.run(4222, dht::crypto::generateIdentity(), true);
        node.bootstrap("bootstrap.jami.net", "4222");

	std::vector<long> testVec;
	std::string testNumber;
	
	output_log.open("output_log.txt");

	std::cout << "1. General test\n";
	std::cout << "2. Rename test\n";
	std::cout << "3. Remove test\n";
	std::cout << "4. Read and Write test\n";
	std::cout << "Which test would you like to run? ";
	std::getline(std::cin, testNumber);

	if (testNumber == "1") {
		/* General test of all functions */
		std::cout << "\nGeneral test on all functions...\n\n";

		auto start = std::chrono::high_resolution_clock::now();
		dht_init_root();
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_mkdir("/A");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_mkdir("/B");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_remove("/A");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_create("/B/hello.txt");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/B");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_write("/B/hello.txt", "hello world\nI LOVE CS179");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);		

		start = std::chrono::high_resolution_clock::now();
		std::string read_string = dht_read("/B/hello.txt");
		std::cout << "OUTPUT OF READ: " << read_string << std::endl;		
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		std::cout << "init_root ran in " << testVec[0] << " milliseconds\n";
		std::cout << "mkdir     ran in " << testVec[1] << " milliseconds\n";
		std::cout << "mkdir     ran in " << testVec[2] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[3] << " milliseconds\n";
		std::cout << "remove    ran in " << testVec[4] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[5] << " milliseconds\n";
		std::cout << "create    ran in " << testVec[6] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[7] << " milliseconds\n";
		std::cout << "write     ran in " << testVec[8] << " milliseconds\n";
		std::cout << "read      ran in " << testVec[9] << " milliseconds\n";
	}
	else if (testNumber == "2") {
		/* testing rename on a FILE... */
		std::cout << "\ntesting rename on a FILE...\n\n";

		auto start = std::chrono::high_resolution_clock::now();
		dht_init_root();
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_mkdir("/C");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_create("/C/hello.txt");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/C");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_rename("/C/hello.txt", "/C/world.txt");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/C");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		std::cout << "init_root ran in " << testVec[0] << " milliseconds\n";
		std::cout << "mkdir     ran in " << testVec[1] << " milliseconds\n";
		std::cout << "create 	ran in " << testVec[2] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[3] << " milliseconds\n";
		std::cout << "rename    ran in " << testVec[4] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[5] << " milliseconds\n";
	}
	else if (testNumber == "3") {
		/* testing multiple remove */
		std::cout << "\nStress testing remove...\n\n";
		
		auto start = std::chrono::high_resolution_clock::now();
		dht_init_root();
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_mkdir("/D");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_remove("/D");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_create("/D/hello.txt");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/D");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);
		
		start = std::chrono::high_resolution_clock::now();
		dht_mkdir("/D");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_create("/D/world.txt");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_readdir("/D");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		std::cout << "init_root ran in " << testVec[0] << " milliseconds\n";
		std::cout << "mkdir     ran in " << testVec[1] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[2] << " milliseconds\n";
		std::cout << "remove    ran in " << testVec[3] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[4] << " milliseconds\n";
		std::cout << "create    ran in " << testVec[5] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[6] << " milliseconds\n";
		std::cout << "mkdir     ran in " << testVec[7] << " milliseconds\n";
		std::cout << "create    ran in " << testVec[8] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[9] << " milliseconds\n";
		std::cout << "readdir   ran in " << testVec[10] << " milliseconds\n";
	}
	else if (testNumber == "4") {
		auto start = std::chrono::high_resolution_clock::now();
		dht_init_root();
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_create("/E.txt");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		std::string read_string = dht_read("/E.txt");
		std::cout << "OUTPUT OF READ: " << read_string << std::endl;
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_write("/E.txt", "hello world\n");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		read_string = dht_read("/E.txt");
		std::cout << "OUTPUT OF READ: " << read_string << std::endl;
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		dht_write("/E.txt", "I LOVE CS179\n");
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);

		start = std::chrono::high_resolution_clock::now();
		read_string = dht_read("/E.txt");
		std::cout << "OUTPUT OF READ: " << read_string << std::endl;
		stop = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
		testVec.push_back(duration);
		
		std::cout << "init_root ran in " << testVec[0] << " milliseconds\n";
		std::cout << "create    ran in " << testVec[1] << " milliseconds\n";
		std::cout << "read      ran in " << testVec[2] << " milliseconds\n";
		std::cout << "write     ran in " << testVec[3] << " milliseconds\n";
		std::cout << "read      ran in " << testVec[4] << " milliseconds\n";
		std::cout << "write     ran in " << testVec[5] << " milliseconds\n";
		std::cout << "read      ran in " << testVec[6] << " milliseconds\n";
	}
	else {
		std::cout << "Invalid test number\n";
	}
}
