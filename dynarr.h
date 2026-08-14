/**
 * @dynarr.h
 * @author Ben Marples
 */

#ifndef DYNARR_IMPL
#define DYNARR_IMPL
//------------------------------------------------------------------------------
// Includes
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

//------------------------------------------------------------------------------
// Structs
typedef struct
{
     size_t len;
     size_t cap;
} Dyn_Header;
//------------------------------------------------------------------------------
// Macros
#define dhdr(arr) ((Dyn_Header *)arr - 1)
#define dlen(arr) (arr ? dhdr(arr)->len : 0)
#define dcap(arr) (arr ? dhdr(arr)->cap : 0)
#define dGrow(arr, n) ((arr) = da_grow((arr), (n), sizeof(*(arr))))
#define dpush(arr, value) (dGrow(arr, 1), (arr)[dhdr(arr)->len++] = (value));
//------------------------------------------------------------------------------
// Function Implementations

static inline void *da_grow(void *arr, const size_t numberOfElements, const size_t elementSize)
{
     size_t newLen = dlen(arr) + numberOfElements;
     size_t newCap = dcap(arr);

     if (newLen > newCap)
     {
          newCap = newCap ? newCap * 2 : 8;
          if (newCap < newLen)
          {
               newCap = newLen;
          }
          DynHeader *newHdr = realloc(arr ? dhdr(arr) : NULL, sizeof(Dyn_Header) + newCap * elementSize);

          newHdr->len = dlen(arr);
          newHdr->cap = newCap;

          return newHdr++;
     }
     return arr;
}

#endif