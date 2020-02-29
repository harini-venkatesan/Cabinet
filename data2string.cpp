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
