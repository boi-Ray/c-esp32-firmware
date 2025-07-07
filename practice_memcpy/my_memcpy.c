#include <stddef.h>
#include <stdio.h>
void *my_memcpy(void *dest, const void *src, size_t n) {       //void *可以回傳任何型別的記憶體位址
    printf("Custom memcpy called!\n");
    unsigned char *d = (unsigned char *)dest;               //C 標準明確規定：「對任意記憶體內容做 bit-level 操作時，應使用 unsigned char*」。
    const unsigned char *s = (const unsigned char *)src;    //const 防止更改來源

    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }

    return dest;
}