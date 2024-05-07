#include <iostream>
#include "PerfEvent.hpp"

int main(){
    int n = 10;

    PerfEvent e;
    e.startCounters();

    for(int i = 0; i < n; i++){
        printf("Hi\n");
    }

    e.stopCounters();
    e.printReport(std::cout, n);
    std::cout << std::endl;
    
    return 0;
}