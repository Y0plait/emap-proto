#include <stdio.h>

#include "types.h"

int main() {
    enum EMAP_Code G_ACK = GENERAL_ACK;
    printf("%d\n", G_ACK);
    return 0;
}