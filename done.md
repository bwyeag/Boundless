
设 $C(n) n\in N$ 为节点数为 $n$ 的二叉树数目
```c
#include <gmp.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    size_t n = 8192;
    mpz_t arr[n];
    mpz_init_set_str(arr[0], "1", 10);
    mpz_init_set_str(arr[1], "1", 10);
    for (size_t i = 2; i < n; i++) {
        mpz_init_set_str(arr[i], "0", 10);
    }
    mpz_t t;
    mpz_init_set_str(t, "0", 10);
    printf("\nstart");
    for (size_t i = 2; i < n; i++) {
        for (size_t j = 1; j <= i; j++) {
            mpz_mul(t, arr[j - 1], arr[i - j]);
            mpz_add(arr[i], arr[i], t);
        }
        printf("%d\n", i);
    }
    FILE* fp = fopen("D:\\c++programs\\vscode\\Boundless\\bigintout.txt", "w");
    if (!fp) {
        printf("Error.");
        return -1;
    }
    else
    {
        printf("Success.");
    }
    for (size_t i = 0; i < n; i++) {
        fprintf(fp,"%d\t",i); 
        mpz_out_str(fp, 10 ,arr[i]);
        fputc('\n',fp);
    }
    for (size_t i = 0; i < n; i++) {
        mpz_clear(arr[i]);
    }
    mpz_clear(t);
}
```