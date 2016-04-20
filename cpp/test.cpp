
#include "MasterQueue.h"
#include <assert.h>

int main() {
    //TEST
    
    MasterQueue q;
    for (int i=0; i<100; i++) {
        Spottings s = q.getBatch(5);
        assert(s.instances.size()<9);
    }
}
