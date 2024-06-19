#ifndef SAMPLER_H
#define SAMPLER_H

enum Sampler {
    /* Reads at the end of the process */
    ONE_SHOT,
    /* Reads both at the start and the end of the process and returns the difference */
    TWO_SHOT,
    /* Reads at an interval */
    POLLING
};


#endif //SAMPLER_H
