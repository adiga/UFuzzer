/* Basic random generator
 * */
#include "common-header.h"
#include "random.h"
#include <time.h>
#include <assert.h>

/* rand function init */
unsigned int rand_init(void){
        unsigned int seed = time(NULL);
        printf("Random generator seed: %u\n", seed);
        srand(seed);
        return seed;
}

/* generate a rand num between 0 and n - 1 */
unsigned int rand_gen(unsigned int n){
        assert(n != 0);
        return rand() % n;
}

