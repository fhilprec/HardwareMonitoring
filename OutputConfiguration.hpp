#pragma once
#include <string>
#include <ostream>
#include <utility>
#include <memory>

struct OutputConfiguration{
    std::string separator;
    bool fileMode;
    std::shared_ptr<std::ostream> stream_;

    OutputConfiguration(std::string separator, bool fileMode, std::shared_ptr<std::ostream> stream) : separator(std::move(separator)),
                                                                                             fileMode(fileMode),
                                                                                             stream_(std::move(stream)) {}
};