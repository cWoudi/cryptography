# Projet R4.B.10

## But 

Écrire des programmes, en C, à l'aide d'une bibliothèque fournie, qui permettent :
- de générer une clef RSA $(n,e,d)$ de taille au moins 1024 bits (2048 est mieux),
- de chiffrer/déchiffrer un entier $m < n$,
- de chiffrer/déchiffrer un fichier.

Il faudra notamment, pour la génération de la clef, utiliser l'algorithme de [**Rabin-Miller**](./RM.md) pour la génération
des nombres premiers.

### Programmes
#### Génération de clef
`key_gen.c`  génére une clef rsa dans deux fichiers (noms passés sur la ligne de commandes) :

- key.pub (n et e)
- key.priv (n, e et d)

Les entiers seront stockés en ligne sous forme de chaînes de caractéres représentant la valeur en hexadécimal.

Exemple de fichier key.pub
```txt
0x9E303E9427CD75BDACB3A7CAA5E2870BC1BEDB24AFE43A72AACA9EFF6E52D880344AE9EE70686146D277CAA6902E3FD363E289C2047D6DD16ADEF5A97402330812EE2755C46BF263CA24A4315E3AE5D912A8261713493208B5A607DE5F1279C366ACEC1C53566134567A463A9A29AC84C4DC2D9C48B1D1D369C8288CD62B9000B159CD206700A9A2DC1D2BE9CAD55F664D664CD82B52659776227DC858C2019885F2E56FDAEA9741BCCDAFFE0630CDD7C614809225734186CEA79660D6626FE8808D8729867E852EA19347236FC5AD75C5F9518AEB37F926B2D6E1F261812DF5E3D4178A28C93FAACB52A879795022FF80758E268DC6792D369689B257D0854B
0x10001
```

#### Chiffrement d'un entier
`crypt_rsa.c` permet de chiffrer/déchiffrer un entier stocké dans un fichier texte en héxadécimal. La commande prend les arguments suivants :

```bash
crypt_rsa -e|-d file.key file.in file.out
```

#### Chiffrement d'un fichier
`crypt_rsa_file.c` permet de chiffrer/déchiffrer un fichier.

```bash
crypt_rsa -e|-d file.key file.in file.out
```
On supposra que le chiffrement/décchiffrement utilise la même endianness.

### Travail à rendre
Le miniprojet est à faire en binôme, pour le 5 avril. Vous rendrez une archive compréssé [ici](https://www.iut-fbleau.fr/auth.php?site=/site/site/DEVOIR/).
Celle-ci contiendra :
- vos programmes, qui doivent utiliser la [bibliothèque](./lib) fournie pour la manipulation de grands nombres entiers
- un `Makefile`.





