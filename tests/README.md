# mod_random Test Suite

Suite de tests complète pour le module Apache mod_random.

## Compilation

```bash
make
```

## Exécution des tests

```bash
make test
# ou directement
./test_mod_random
```

## Couverture des tests

### Tests d'encodage (7 tests)
- `test_hex_encoding_basic` - Encodage hexadécimal basique
- `test_hex_encoding_empty` - Encodage de données vides
- `test_hex_encoding_single_byte` - Encodage d'un seul byte
- `test_hex_encoding_test_vectors` - Vecteurs de test connus
- `test_base64url_encoding_basic` - Encodage base64url
- `test_custom_alphabet_basic` - Alphabet personnalisé
- `test_custom_alphabet_with_grouping` - Alphabet avec groupement

### Tests de génération aléatoire (9 tests)
- `test_generate_string_hex` - Génération format hex
- `test_generate_string_base64` - Génération format base64
- `test_generate_string_base64url` - Génération format base64url
- `test_generate_string_custom_alphabet` - Génération avec alphabet personnalisé
- `test_token_length_minimum` - Validation longueur minimale
- `test_token_length_maximum` - Validation longueur maximale
- `test_token_uniqueness` - Unicité probabilistique (100 tokens)
- `test_random_generation_different` - Différence entre générations
- `test_large_token_generation` - Génération de grands tokens (256 bytes)

### Tests cryptographiques (3 tests)
- `test_hmac_sha256_basic` - HMAC-SHA256 basique
- `test_hmac_sha256_consistency` - Cohérence HMAC (même entrée = même sortie)
- `test_hmac_sha256_different_keys` - Clés différentes = sorties différentes

### Tests infrastructure APR (4 tests)
- `test_thread_mutex_basic` - Création et verrouillage de mutex
- `test_pool_allocation` - Allocation de pools APR
- `test_apr_psprintf_basic` - Concaténation de chaînes
- `test_time_functions` - Fonctions de temps APR

### Tests de validation (2 tests)
- `test_constants_validation` - Validation des constantes (sentinelles, limites)
- `test_format_enum_values` - Valeurs d'énumération de format

## Total : 25 tests

Tous les tests vérifient :
- ✅ Encodage hexadécimal (minuscules)
- ✅ Encodage base64 standard
- ✅ Encodage base64url (URL-safe, sans padding)
- ✅ Alphabets personnalisés avec groupement optionnel
- ✅ CSPRNG (génération cryptographiquement sûre)
- ✅ HMAC-SHA256 pour signatures
- ✅ Thread-safety avec mutexes APR
- ✅ Gestion mémoire avec pools APR
- ✅ Validation des constantes et sentinelles
- ✅ Limites de longueur de tokens

## Dépendances

- APR (Apache Portable Runtime)
- APR-Util
- OpenSSL (pour HMAC-SHA256)
- Apache httpd headers

## Notes

- Les tests utilisent un framework de test simple (macros TEST, ASSERT_*)
- Chaque test est autonome et utilise un pool APR propre
- Les tests de CSPRNG vérifient l'unicité probabilistique
- Les vecteurs de test connus valident l'exactitude des encodages
