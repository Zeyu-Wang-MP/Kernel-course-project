#include <iostream>
#include <string>
#include "thread.h"

void C(void *a) {
    std::string s = thread::swap3("message #1 from C");
    std::cout << "C received: " << s << std::endl;

    s = thread::swap3("message #2 from C");
    std::cout << "C received: " << s << std::endl;
}

void B(void *a) {
    std::string s = thread::swap3("message #1 from B");
    std::cout << "B received: " << s << std::endl;

    s = thread::swap3("message #2 from B");
    std::cout << "B received: " << s << std::endl;
}

void A(void *a) {
    thread B_thread (B, nullptr);
    thread C_thread (C, nullptr);

    std::string s = thread::swap3("message #1 from A");
    std::cout << "A received: " << s << std::endl;

    thread::yield();

    s = thread::swap3("message #2 from A");
    std::cout << "A received: " << s << std::endl;
}

int main() {
    cpu::boot(1, A, nullptr, false, false, 0);
}