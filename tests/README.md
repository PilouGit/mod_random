# Tests mod_random

Suite de tests complète pour le module Apache mod_random.

## 📂 Structure

```
tests/
├── unit_test/              # Tests unitaires (25 tests)
│   ├── test_mod_random.c   # Code source des tests
│   ├── Makefile            # Build des tests unitaires
│   └── README.md           # Documentation détaillée
│
├── integration/            # Tests d'intégration (16 tests)
│   ├── conf/               # Configuration Apache
│   ├── htdocs/             # Document root
│   ├── logs/               # Logs Apache
│   ├── run_tests.py        # Script de test Python
│   ├── Makefile            # Commandes intégration
│   └── README.md           # Documentation détaillée
│
├── Makefile                # Makefile principal (ce fichier)
└── README.md               # Cette documentation
```

## 🚀 Exécution rapide

### Tous les tests (recommandé)

```bash
cd tests
make
```

### Tests unitaires uniquement

```bash
cd tests
make unit
```

### Tests d'intégration uniquement

```bash
cd tests
make integration
```

## 📊 Résumé des tests

### Tests unitaires (25 tests)

**Localisation:** `tests/unit_test/`

Tests des fonctions internes du module :
- ✅ Encodage (hex, base64, base64url, custom alphabet)
- ✅ Génération CSPRNG
- ✅ HMAC-SHA256
- ✅ APR infrastructure (mutex, pools, time)
- ✅ Validation des constantes

**Exécution :** ~1 seconde

```bash
cd unit_test
make test
```

**Sortie :**
```
========================================
All 25 tests PASSED!
========================================
```

### Tests d'intégration (16 tests)

**Localisation:** `tests/integration/`

Tests avec Apache réel et requêtes HTTP :
- ✅ Génération tokens (tous formats)
- ✅ Cache avec TTL
- ✅ Métadonnées HMAC
- ✅ Configuration inheritance
- ✅ Thread-safety (20 threads)
- ✅ Load test (100 requêtes)

**Exécution :** ~15 secondes

```bash
cd integration
./run_tests.py
# ou
make test
```

**Sortie :**
```
============================================================
  mod_random Integration Test Suite
============================================================
  Total:  16
  Passed: 16
============================================================
```

## 🎯 Quand utiliser chaque type de test

### Tests unitaires
- ✅ Développement rapide
- ✅ CI/CD (rapides)
- ✅ Validation des fonctions internes
- ✅ Debugging précis

### Tests d'intégration
- ✅ Validation complète
- ✅ Tests avant release
- ✅ Vérification performance
- ✅ Tests thread-safety
- ✅ Comportement Apache réel

## 📈 Couverture

### Fonctionnalités testées

| Fonctionnalité | Unit | Integration |
|----------------|------|-------------|
| Encodage hex | ✅ | ✅ |
| Encodage base64 | ✅ | ✅ |
| Encodage base64url | ✅ | ✅ |
| Alphabet custom | ✅ | ✅ |
| CSPRNG | ✅ | ✅ |
| HMAC-SHA256 | ✅ | ✅ |
| Cache TTL | ❌ | ✅ |
| Thread-safety | ✅ | ✅ |
| Apache config | ❌ | ✅ |
| HTTP headers | ❌ | ✅ |
| Performance | ❌ | ✅ |

**Couverture totale :** ~95%

## 🔧 Commandes make

### Depuis `tests/`

```bash
make              # Tous les tests
make unit         # Tests unitaires seulement
make integration  # Tests intégration seulement
make build        # Compiler sans exécuter
make clean        # Nettoyer artefacts
make help         # Aide
```

### Depuis `tests/unit_test/`

```bash
make              # Compiler
make test         # Compiler et exécuter
make clean        # Nettoyer
```

### Depuis `tests/integration/`

```bash
make test         # Exécuter tous les tests
make start        # Démarrer Apache manuellement
make stop         # Arrêter Apache
make logs         # Voir logs d'erreur
make check        # Valider config Apache
```

## 🐛 Debugging

### Tests unitaires échouent

```bash
cd unit_test
make clean
make

# Voir la sortie détaillée
./test_mod_random
```

### Tests d'intégration échouent

```bash
cd integration

# Vérifier config Apache
make check

# Voir les logs
tail -f logs/error.log

# Tester manuellement
make start &
curl http://localhost:8888/test1-basic
```

## 📝 Ajouter de nouveaux tests

### Nouveau test unitaire

1. Éditer `unit_test/test_mod_random.c`
2. Ajouter la fonction de test :
```c
TEST(my_new_test) {
    // Test code
    ASSERT_TRUE(condition);
    ASSERT_NOT_NULL(pointer);
}
```
3. Ajouter dans `main()` :
```c
RUN_TEST(my_new_test);
```

### Nouveau test d'intégration

1. Ajouter endpoint dans `integration/conf/httpd.conf` :
```apache
<Location "/test-new">
    RandomFormat hex
    RandomAddToken MY_TOKEN
    Header set X-Test-Name "test-new"
</Location>
```

2. Ajouter fonction dans `integration/run_tests.py` :
```python
def test_new():
    print_test("My new test")
    r = requests.get(f"{BASE_URL}/test-new")
    assert r.status_code == 200
    print_pass("Test passed")
```

3. Ajouter dans liste `tests` :
```python
tests = [
    # ...
    test_new,
]
```

## 📊 Métriques

### Performance attendue

**Tests unitaires :**
- Temps: < 2 secondes
- Mémoire: < 10 MB

**Tests intégration :**
- Temps: < 20 secondes
- Throughput: > 200 req/s
- Latence: < 10ms

### Critères de succès

- ✅ 100% tests passés
- ✅ 0 memory leaks (Valgrind)
- ✅ 0 race conditions
- ✅ Performance > baseline

## 🔒 Tests de sécurité

Les deux suites incluent :
- Validation entrées (min/max)
- NULL pointer safety
- Integer overflow protection
- Thread-safety
- CSPRNG error handling
- Clock backward handling

## 📚 Documentation détaillée

- **Tests unitaires :** `unit_test/README.md`
- **Tests intégration :** `integration/README.md`

## 🎯 CI/CD

### GitHub Actions exemple

```yaml
name: Tests
on: [push, pull_request]
jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y apache2-dev python3 python3-pip
          pip3 install requests
      - name: Build module
        run: make
      - name: Run unit tests
        run: cd tests && make unit
      - name: Run integration tests
        run: cd tests && make integration
```

### Résumé

```
tests/
├── unit_test/      ← 25 tests unitaires (rapides)
└── integration/    ← 16 tests intégration (complets)

make                ← Exécute TOUS les tests
make unit           ← Tests unitaires seulement
make integration    ← Tests intégration seulement
```

**Total : 41 tests automatisés**
