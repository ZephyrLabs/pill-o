#include "main.hpp"

using namespace display;

int main(){

    std::thread display_thread (display_init);
    
    while(1){}
    
    return 0;
}
