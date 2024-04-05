# Rabin-Miller Test

If $p>2$ is a prime number, we can write $p-1 = 2^r \times d$, where $r\geq 1$ and $d$ is odd.

According to Fermat's little theorem, if $1\leq a < p$, then $a^{p-1} = a^{2^r d} = 1\mod p$.

Therefore, $a^{2^{r-1}d}$ is a solution of the equation $x^2 = 1 \mod p$. Since $p$ is prime, this equation has
exactly $2$ solutions: $-1$ and $1$.

Ainsi, 
$$
a^{2^{r-1}d} \equiv \pm 1 \mod p
$$


By induction, it is shown that we are necessarily in one of the two situations:
- $a^d = 1 \mod p$
- there exists an integer $i$, $0\leq i\leq r-1$ such that $a^{2^i d} = -1 \mod p$

Therefore, if both propositions are false, $p$ is not prime.

## Implementation Notes
- To write $p-1 = 2^r \times d$, we just need a bit shift.
- For the Rabin test, we need a function to compute a modular exponentiation, which you
  will also need for RSA.
- To verify that a number is probably prime, randomly draw some numbers $a$ (up to you) 
  to be tested with Rabin-Miller.
- To generate a prime number on $b$ bits, one can randomly draw numbers (odd ones!) from the interval
   $[2^{b-1} +1,2^b -1]$ until finding one that passes the Rabin test. It's also possible to iterate through the interval.
- To accelerate the search, one can include in the Rabin test a divisibility check with a set of small prime factors.
