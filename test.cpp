#include <opendht.h>
#include <vector>
#include<unistd.h>
#include<iostream>
#include <fstream>
#include <string>


int main()
{
    dht::DhtRunner node;
    std::ofstream myfile;

    myfile.open("example.txt");
    myfile <<"This is a test file\n";
    myfile.close();
    // Launch a dht node on a new thread, using a
    // generated RSA key pair, and listen on port 4222.
    node.run(4855, dht::crypto::generateIdentity(), true);

    // Join the network through any running node,
    // here using a known bootstrap node.
    node.bootstrap("localhost", "4222");

    // put some data on the dht
	
	std::ifstream ifs("example.txt");
	std::string content( (std::istreambuf_iterator<char>(ifs) ),
				(std::istreambuf_iterator<char>() ) );

    node.put("unique_key", content);

    // put some data on the dht, signed with our generated private key
    //node.putSigned("unique_key_42", some_data);

   
    // get data from the dht

    node.get("unique_key", [](const std::vector<std::shared_ptr<dht::Value>>& values) {
        // Callback called when values are found
        for (const auto& value : values)
            std::cout << "Found value: " << *value << std::endl;
        return true; // return false to stop the search
    });


    // wait for dht threads to end

    sleep(200);
    node.join();

    return 0;
}
