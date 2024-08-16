#ifndef MEASUREMENT_H
#define MEASUREMENT_H
#include <string>

struct Measurement {
    Measurement() = default;
    explicit Measurement(std::string value) : value(std::move(value)) {}
    std::string value;
};

#endif //MEASUREMENT_H
