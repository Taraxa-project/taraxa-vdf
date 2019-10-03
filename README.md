# Verifiable delay function implementations
I implement the VDF protocols from Wesolowski [1] and Pietrzak [2] in C/C++, using both OpenSSL and GNU MP for handling large integers.
I employ OpenSSL for its cryptographic features and GNU MP for its more user-friendly RNG.

The random oracle that hashes into a large set of prime numbers -- needed in the Wesolowski protocol -- is implemented using acceptance-rejection sampling.

## References
[1] Benjamin Wesolowski. "Efficient verifiable delay functions." IACR Cryptology ePrint Archive 2018 (2018): 623.

[2] Krzysztof Pietrzak. "Simple verifiable delay functions." 10th Innovations in Theoretical Computer Science Conference (ITCS 2019). Schloss Dagstuhl-Leibniz-Zentrum fuer Informatik, 2018.

[3] Dan Boneh, Benedikt Bünz, Ben Fisch. "A Survey of Two Verifiable Delay Functions." Cryptology ePrint Archive, 2018. https://crypto.stanford.edu/~dabo/pubs/papers/VDFsurvey.pdf


## Mac Need to brew install openssl@1.1
```
For compilers to find openssl@1.1 you may need to set:
  export LDFLAGS="-L/usr/local/opt/openssl@1.1/lib"
  export CPPFLAGS="-I/usr/local/opt/openssl@1.1/include"

For pkg-config to find openssl@1.1 you may need to set:
  export PKG_CONFIG_PATH="/usr/local/opt/openssl@1.1/lib/pkgconfig"
```
