# Tests d'intégration mod_random

Suite de tests d'intégration complète pour le module Apache mod_random.

## ⚙️ Configuration automatique

La configuration Apache (`conf/httpd.conf`) est **générée automatiquement par CMake** à partir du template `httpd.conf.in`. Les chemins sont automatiquement ajustés selon votre environnement.

**Important :**
- ✅ Modifiez `httpd.conf.in` (template)
- ❌ Ne modifiez **jamais** `httpd.conf` directement (fichier généré)
- Exécutez `cmake .` à la racine du projet pour régénérer la configuration

### Variables CMake utilisées :
- `@TEST_INTEGRATION_DIR@` → Répertoire des tests d'intégration
- `@APACHE_MODULE_DIR@` → Répertoire des modules Apache
- `@MOD_RANDOM_PATH@` → Chemin du module mod_random.so

## 📋 Prérequis

```bash
# Apache2
sudo apt-get install apache2-dev

# Python 3 + requests
pip3 install requests
```

## 🚀 Exécution rapide

```bash
cd tests/integration
chmod +x run_tests.py
./run_tests.py
```

## 📂 Structure

```
tests/integration/
├── conf/
│   ├── httpd.conf.in       # Template de configuration (ÉDITER CELUI-CI)
│   └── httpd.conf          # Configuration générée par CMake (NE PAS ÉDITER)
├── htdocs/
│   ├── index.html          # Document root
│   └── test.html           # Page de test pour les endpoints
├── logs/                   # Logs Apache (créé automatiquement)
│   ├── error.log
│   └── httpd.pid
├── Makefile                # Commandes make pour les tests
├── run_tests.py            # Script de test principal
└── README.md               # Ce fichier
```

## 🧪 Tests couverts

### Test 1: Génération basique
- Endpoint: `/test1-basic`
- Vérifie la génération de token par défaut

### Test 2: Format hexadécimal
- Endpoint: `/test2-hex`
- Format: `RandomFormat hex`
- Vérifie tokens en hexadécimal

### Test 3: Format base64url
- Endpoint: `/test3-base64url`
- Format: `RandomFormat base64url`
- Longueur: 32 bytes
- Vérifie format URL-safe

### Test 4: Alphabet personnalisé
- Endpoint: `/test4-custom`
- Alphabet: `0123456789ABCDEF`
- Grouping: 4 caractères
- Vérifie alphabet custom avec groupement

### Test 5: Token avec timestamp
- Endpoint: `/test5-timestamp`
- Option: `RandomIncludeTimestamp on`
- Vérifie inclusion timestamp

### Test 6: Prefix et suffix
- Endpoint: `/test6-prefix-suffix`
- Prefix: `csrf_`
- Suffix: `_v1`
- Vérifie ajout prefix/suffix

### Test 7: Cache avec TTL
- Endpoint: `/test7-cache`
- TTL: 5 secondes
- **Vérifie:**
  - Premier appel → génération
  - Deuxième appel immédiat → cache hit
  - Après 6 secondes → cache expiration

### Test 8: Tokens multiples
- Endpoint: `/test8-multiple`
- Génère 3 tokens différents
- Vérifie génération multiple

### Test 9: Token dans header HTTP
- Endpoint: `/test9-header`
- Output: Variable + Header `X-CSRF-Token`
- Vérifie sortie dans header

### Test 10: Encodage métadonnées
- Endpoint: `/test10-metadata`
- Encodage avec HMAC-SHA256
- Expiry: 3600 secondes
- Signing key configurée

### Test 11: Filtrage URL pattern
- Endpoint: `/test11-pattern/api/*`
- Pattern: `^/test11-pattern/api/`
- **Vérifie:**
  - `/api/endpoint` → token généré
  - `/other` → pas de token

### Test 12: Longueur minimale
- Endpoint: `/test12-minlength`
- Longueur: 1 byte
- Vérifie validation min

### Test 13: Longueur maximale
- Endpoint: `/test13-maxlength`
- Longueur: 1024 bytes
- Vérifie validation max

### Test 14: Héritage configuration
- Endpoint parent: `/test14-inherit`
- Endpoint enfant: `/test14-inherit/child`
- **Vérifie:**
  - Parent: hex, 16 bytes
  - Enfant override: base64url, 32 bytes

### Test 15: Stress test cache
- Endpoint: `/test15-cache-stress`
- 20 requêtes concurrentes
- **Vérifie:**
  - Thread-safety du cache
  - Pas de race conditions
  - Performances sous charge

### Test 16: Load test serveur
- 100 requêtes rapides séquentielles
- Mesure throughput (req/s)
- Vérifie stabilité

## 📊 Sortie des tests

```
============================================================
  mod_random Integration Test Suite
============================================================

Starting Apache test server...
  ✓ Apache started successfully

[TEST] Basic token generation
  ✓ Test endpoint correct
  ✓ Basic test passed

[TEST] Cache with TTL (5 seconds)
  ✓ First request successful
  ✓ Second request successful (cached)
  ℹ Waiting 6 seconds for cache expiration...
  ✓ Third request successful (cache expired)

...

============================================================
  Test Summary
============================================================
  Total:  16
  Passed: 16
============================================================
```

## 🔧 Configuration manuelle

### Démarrer Apache manuellement

```bash
cd tests/integration
apache2 -f $(pwd)/conf/httpd.conf -DFOREGROUND
```

### Tester manuellement avec curl

```bash
# Test basique
curl -v http://localhost:8888/test1-basic

# Test avec cache
curl -v http://localhost:8888/test7-cache

# Test format hex
curl -v http://localhost:8888/test2-hex

# Test header output
curl -v http://localhost:8888/test9-header 2>&1 | grep X-CSRF-Token
```

## 🐛 Debugging

### Voir les logs Apache

```bash
# Logs d'erreur
tail -f tests/integration/logs/error.log

# Logs d'accès
tail -f tests/integration/logs/access.log
```

### Augmenter le niveau de log

Modifier `conf/httpd.conf`:
```apache
LogLevel trace8  # Maximum verbosity
```

### Vérifier le module chargé

```bash
apache2 -f $(pwd)/conf/httpd.conf -t -D DUMP_MODULES | grep random
```

## ⚙️ Personnalisation

### Ajouter un nouveau test

1. **Ajouter configuration dans `conf/httpd.conf`:**
```apache
<Location "/test-custom">
    RandomFormat hex
    RandomLength 24
    RandomAddToken MY_TOKEN
    Header set X-Test-Name "test-custom"
</Location>
```

2. **Ajouter fonction de test dans `run_tests.py`:**
```python
def test_custom():
    """Test custom: Description"""
    print_test("Custom test description")

    r = requests.get(f"{BASE_URL}/test-custom")
    assert r.status_code == 200
    assert r.headers.get('X-Test-Name') == 'test-custom'

    print_pass("Custom test passed")
```

3. **Ajouter dans la liste des tests:**
```python
tests = [
    # ...
    test_custom,
]
```

## 📝 Notes

- Les tests sont **non-destructifs** et isolés
- Apache écoute sur le port **8888** (configurable)
- Les logs sont dans `logs/`
- Le serveur est automatiquement arrêté après les tests
- Compatibilité: Python 3.6+

## 🚨 Problèmes courants

### Port 8888 déjà utilisé

Modifier dans `conf/httpd.conf`:
```apache
Listen 9999  # Autre port
```

Et dans `run_tests.py`:
```python
BASE_URL = "http://localhost:9999"
```

### Permission denied

```bash
# Donner droits exécution
chmod +x run_tests.py

# Ou exécuter avec python3
python3 run_tests.py
```

### Module non trouvé

Vérifier le chemin dans `conf/httpd.conf`:
```apache
LoadModule random_module /path/to/mod_random.so
```

### Apache ne démarre pas

```bash
# Tester la configuration
apache2 -f $(pwd)/conf/httpd.conf -t

# Voir les erreurs
cat logs/error.log
```

## 📈 Métriques

Les tests mesurent:
- **Temps de réponse** moyen
- **Throughput** (requêtes/seconde)
- **Cache hit ratio**
- **Concurrence** (20 threads simultanés)
- **Stabilité** (100 requêtes consécutives)

## 🎯 Objectifs de performance

- ✅ Throughput: > 1000 req/s
- ✅ Latence: < 10ms par requête
- ✅ Cache: 100% hit rate dans TTL
- ✅ Concurrence: 0 erreurs sur 20 threads
- ✅ Stabilité: 100% success sur 100 requêtes

## 🔐 Tests de sécurité

Les tests vérifient également:
- Pas de crash avec valeurs limites
- Thread-safety du cache
- Validation des entrées
- Gestion erreurs CSPRNG
- Protection overflow

## 📚 Ressources

- [Apache Testing Guide](https://httpd.apache.org/test/)
- [mod_random Documentation](../../README.md)
- [Unit Tests](../README.md)
