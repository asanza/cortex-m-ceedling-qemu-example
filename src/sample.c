#include "sample.h"

int sum(int a, int b) {
    for (int i = 0; i < 100; i++) {
        a += i;
    }
    if( b > 2 )
        return a + b;
    else
        return a - b;
    return a + b;
}