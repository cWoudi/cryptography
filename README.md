Write programs, in C, using a provided library, which allow:
- to generate an RSA key $(n,e,d)$ of size at least 1024 bits (2048 is better),
- to encrypt/decrypt an integer $m < n$,
- to encrypt/decrypt a file.
  

In particular, for the generation of the key, it will be necessary to use the Rabin-Miller algorithm for the generation of prime numbers.


### **Programs**


#### Key generation : 
`key_gen.c` generates an rsa key in two files (names passed on the command line):
- key.pub ($n$ and $e$)
- key.priv ($n, e$ and $d$)
The integers will be stored online as strings representing the value in hexadecimal.


Example of `key.pub` file :

```
0x9E303E9427CD75BDACB3A7CAA5E2870BC1BEDB24AFE43A72AACA9EFF6E52D880344AE9EE70686146D277CAA6902E3FD363E289C2047D6DD16ADEF5A97402330812EE2755C46BF263CA24A4315E3AE5D912A8261713493208B5A607DE5F1279C366ACEC1C53566134567A463A9A29AC84C4DC2D9C48B1D1D369C8288CD62B9000B159CD206700A9A2DC1D2BE9CAD55F664D664CD82B52659776227DC858C2019885F2E56FDAEA9741BCCDAFFE0630CDD7C614809225734186CEA79660D6626FE8808D8729867E852EA19347236FC5AD75C5F9518AEB37F926B2D6E1F261812DF5E3D4178A28C93FAACB52A879795022FF80758E268DC6792D369689B257D0854B
0x10001
```


#### Encrypting an integer
`crypt_rsa.c` allows you to encrypt/decrypt an integer stored in a hexadecimal text file. The command takes the following arguments:
```
crypt_rsa -e|-d file.key file.in file.out
```


#### Encrypting a file
`crypt_rsa_file.c` allows you to encrypt/decrypt a file.  The command takes the following arguments:
```
crypt_rsa -e|-d file.key file.in file.out
```


We will assume that the encryption/decryption uses the same endianness.


#### Work to be done : 
- your programs, which must use the provided library for handling large integers
- a `Makefile`
