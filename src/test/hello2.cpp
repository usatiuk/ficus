#include <iostream>

void badfn() {
    throw std::runtime_error("oops");
}

int main() {
    std::cout << "hi" << std::endl;
    try {
        std::cout << "try" << std::endl;
        badfn();
        std::cout << "ok" << std::endl;
    } catch (std::exception &e) {
        std::cout << "huh" << std::endl;
        std::cout << e.what() << std::endl;
    }
    std::cout << "fin" << std::endl;
}