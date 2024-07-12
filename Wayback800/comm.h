#include <unistd.h>

const uint32_t spdc1016freq=3686400*1;

const uint32_t num_batch_per_second=50;

const uint32_t ms_per_batch=1000/num_batch_per_second;

const uint32_t num_cycle_per_batch=spdc1016freq/num_batch_per_second;

unsigned int GetTickCount();
