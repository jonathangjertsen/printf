# printf

This is a partial printf implementation.

Features:

* runtime configurable "stdio"
* prints as many characters as it can at once
* binary formatting (%b)
* features can be turned on and off

Non-features

* float
* padding
* obscure stuff

## Integration

Definitions for the following functions must be defined:

```C
// Called before printing.
// Returns a function that emits to the output channel, or NULL if none is available in which case the print is skipped.
emit_func_t printf_claim_stdio();

// Called after printing (unless printf_claim_stdio() returned NULL)
void printf_release_stdio(emit_func_t stdio_func);
```

where `stdio_func` is a function with the signature 

```C
void emit(const char *buffer, int size);
```


## Flags

Here is an exhaustive list of all the supported flags:

specifier | datatype | formatting | compiler flags required
---|---|---|---
%d | int | decimal | none
%i | int |decimal | none
%u | unsigned int | decimal | none
%x | unsigned int | hex | none
%#x | unsigned int | hex with prefix | none
%#b | unsigned int | binary with prefix | none
%s | string |  | none
%% | literal '%' | | none
%ld | long int | decimal | FEATURE_32BIT
%li | long int |decimal | FEATURE_32BIT
%lu | long unsigned int | decimal | FEATURE_32BIT
%lx | long unsigned int | hex | FEATURE_32BIT
%#lx | long unsigned int | hex with prefix | FEATURE_32BIT
%lb | long unsigned int | binary | FEATURE_32BIT 
%#lb | long unsigned int | binary with prefix | FEATURE_32BIT 
%lld | long long int | decimal | FEATURE_32BIT & FEATURE_64BIT
%lli | long long int |decimal | FEATURE_32BIT & FEATURE_64BIT
%llu | long long unsigned int | decimal | FEATURE_32BIT & FEATURE_64BIT
%llx | long long unsigned int | hex | FEATURE_32BIT & FEATURE_64BIT
%#llx | long long unsigned int | hex with prefix | FEATURE_32BIT & FEATURE_64BIT
%llb | long long unsigned int | binary | FEATURE_32BIT & FEATURE_64BIT
%#llb | long long unsigned int | binary with prefix | FEATURE_32BIT & FEATURE_64BIT
%hd | signed short | decimal | FEATURE_SHORT
%hi | signed short | decimal | FEATURE_SHORT
%hu | unsigned short | decimal | FEATURE_SHORT
%hx | unsigned short | hex | FEATURE_SHORT
%#hx | unsigned short | hex with prefix | FEATURE_SHORT
%hb | unsigned short | binary | FEATURE_SHORT
%#hb | unsigned short | binary with prefix | FEATURE_SHORT
%hhd | char | decimal | FEATURE_SHORT & FEATURE_CHAR
%hhx | char | hex | FEATURE_SHORT & FEATURE_CHAR
%#hhx | char | hex with prefix | FEATURE_SHORT & FEATURE_CHAR
%hhb | char | binary | FEATURE_SHORT & FEATURE_CHAR
%#hhb | char | binary with prefix | FEATURE_SHORT & FEATURE_CHAR
%c | char | ascii | FEATURE_SHORT & FEATURE_CHAR 
