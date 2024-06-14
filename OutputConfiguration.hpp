#include <string>
#include <ostream>

class OutputConfiguration{
public:
    std::string *seperator;
    bool fileMode;
    std::ostream *stream_;

    OutputConfiguration(std::string &seperator, bool fileMode, std::ostream &stream_){
        this->seperator = seperator;
        this->fileMode = fileMode;
        this->stream_ = stream_;
    }
};