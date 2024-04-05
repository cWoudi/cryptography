# bign 1.05
This library allows for arithmetic operations with arbitrary precision. Only the part about integer calculation 
is of interest to us.

The basic type is `big_n`. It is an array of elements (`int`) that enables block arithmetic.

The array size is statically managed by the `PREC` constant. You will need to modify it and recompile the library according to your 
needs.

With declaration

```c
big_n n;
```

`n` is an `int` array of size `PREC + 1`. One element is added, and it equals 1 if the integer is negative.
The least significant block is placed in `n[0]`.

```txt
n[0] n[1] ... n[PREC-1] n[PREC]
                           ^
                           |
                         sign
```


The library contains all the basic arithmetic functions you will need.
