#include <iostream>
#include "PerfEvent.hpp"
#include "CSVWriter.h"
#include "Counter.hpp"
//sysctl -w kernel.perf_event_paranoid=-1
//g++ test.cpp ; ./a.out ; rm a.out;


int main(){

    Counter c;
}

/*
int main(){
    int a = 7;
    int n = 10000;
    PerfEvent e;
    e.startCounters();
    for (int i=0; i<n; i++) // this code will be measured
    a = a * a;
    e.stopCounters();
    e.printReport(std::cout, n); // use n as scale factor
    std::cout << std::endl;

    CSVWriter csv;
    csv.newRow() << "this" << "is" << "the" << "first" << "row";
    csv.newRow() << "this" << "is" << "the" << "second" << "row";
    csv.writeToFile("foobar.csv");
    return 0;
}*/