#ifndef SAMPLER_H
#define SAMPLER_H
#include <string>
#include <algorithm>

enum SamplingMethod {
    /* Reads at the end of the process */
    ONE_SHOT = 0,
    /* Reads both at the start and the end of the process and returns the difference */
    TWO_SHOT = 1,
    /* Reads at an interval */
    POLLING = 2,
    /* Is not read but calculated afterward */
    CALCULATED = 3
};

static const std::string EnumStrings[] = { "One Shot", "Two Shot", "Polling", "Calculated" };
static std::string getDisplayForSampler(const int enumIndex)
{
    return EnumStrings[enumIndex];
}

static SamplingMethod getSamplerFromDisplayName(const std::string& enumString)
{
    return static_cast<SamplingMethod>(std::distance(std::begin(EnumStrings),
                                                     std::find(std::begin(EnumStrings), std::end(EnumStrings),
                                                               enumString)));
}

#endif //SAMPLER_H
