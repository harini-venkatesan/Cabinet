#include <iostream>
#include <vector>

/* Converts vector of integers to ASCII string */
std::string asciify(std::vector<int> const &vec) {
    std::string fullString;
    
    for (int i = 0; i < vec.size(); i++) {
        fullString += static_cast<char>(vec[i]);
    }
    
    return fullString;
}


int main(){
    /* for test purposes */
    std::vector<int> input = {49,50,51,65,66,67,97,98,99};
    
    std::cout << asciify(input) << std::endl;

    return 0;
}

#include <opendht.h>
#include <sstream>
#include <string>

dht::DhtRunner node;


void foo() {
	std::string aString = "1234abcd";
	dht::Value valueString(aString);
	
	node.put("unique_key", aString);

	sleep(1);
}


void bar() {
		
	node.get("unique_key", [](const std::vector<std::shared_ptr<dht::Value>>& values) {
        // Callback called when values are found
        for (const auto& value : values){
		//dht::Value tempValue;
		//std::cout << typeid((*value).data[0]).name() << std::endl; 
		//tempValue = *value;
		for(int i = 0; i < (*value).data.size(); i++){
			
			printf("%c\n",static_cast<char>(static_cast<int>((*value).data.at(i))));
		}
	}
        return true; // return false to stop the search
    	});
	sleep(1);
}

int main() {
	node.run(4222, dht::crypto::generateIdentity(), true);
	node.bootstrap("bootstrap.jami.net", "4222");

	foo();
	bar();
	return 0;

}
