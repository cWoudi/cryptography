# Test de Rabin-Miller

Si $p>2$ est un nombre premier, on peut écrire $p-1 = 2^r \times d$, avec $r\geq 1$ et $d$ impair.

D'après le petit théorème de Fermat, si $1\leq a < p$, alors $a^{p-1} = a ^{2^r d} = 1\mod p$.

$a^{2^{r-1}d}$ est donc une solution de l'équation $x^2 = 1 \mod p$. Comme $p$ est premier, cette équation admet
	exactement $2$ solutions : $-1$ et $1$.

Ainsi, 
\[ a^{2^{r-1}d} = \pm 1\mod p \]

Par réccurence, on montre qu'on est forcément dans une des deux situtations :
- $a^d = 1 \mod p$
- il existe un entier i, $0\leq i\leq r-1$ tel que $a^{2^i d} = -1 \mod p$

Ainsi, si les deux propositions sont fausses, $p$ n'est pas premier.

## Remarques pour l'implantation 
- Pour écrire $p-1 = 2^r \times d$, on a juste besoin de décalage.
- Pour le test de Rabin, on a besoin d'une d'une fonction qui permet de calculer une exponentielle modulaire, dont vous
  aurez besoin également pour RSA.
- Pour vérifier qu'un nombre est probablement premier, on tire aléatoirement quelques nombres $a$ (à vous de voir)
  pour être testé avec Rabin-Miller.
- Pour générer un nombre premier sur $b$ bits on peut tirer aléatoirement des nombres (impairs!) de l'intervalle
   $[2^{b-1} +1,2^b -1]$ jusqu'à en trouver un qui passe le test de Rabin. On peut aussi itérer l'intervalle.
- Pour accélerer la recherche, on peut include dans le test de Rabin un test de divisibilié avec un ensemble de petits facteurs premiers.
