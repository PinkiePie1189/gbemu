#include <stdio.h>

int foo(int b) {
    return b;
}


int main() {
    int b = 0;
    printf("b: %d\n", b);
    printf("foo: %d\n", foo(b++));
    printf("b: %d\n", b);
    return 0;
}
