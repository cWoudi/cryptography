# bign 1.05
Cette bibliothèque permet de faire du calcul avec une précision arbitraire. Seule la partie sur le calcul entier 
nous interesse.

Le type de base est `big_n`. Il s'agit d'un tableau d'éléments (`int`) qui permet de faire de l'arithmétique par bloc.

La taille du tableau est gérée statiquement par la constante `PREC`. Il vous faudra la modifier et recompiler la bibliothèque suivant vos 
besoins. 

Avec déclaration

```c
big_n n;
```

`n` est un tableau d'int de taille `PREC + 1`. 1 élément est rajouté, et vaut 1 si l'entier est négatif.
Le bloc de poids faible est placé dans `n[0]`.

```txt
n[0] n[1] ... n[PREC-1] n[PREC]
                           ^
						   |
						 signe
```


La bibliothèque contient toutes les fonctions arithmétiques de base dont vous aurez besoin.
