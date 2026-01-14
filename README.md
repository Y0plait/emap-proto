# EMAP

[![pipeline status](https://gitlab.com/sco-chartreux/3ics-25-26/moulin-anton/projet-c/emap/badges/main/pipeline.svg)](https://gitlab.com/sco-chartreux/3ics-25-26/moulin-anton/projet-c/emap/-/commits/main)

## Présentation

EMAP est un projet système développé dans le cadre du cours C Système (S5).

## Documentation

Pour la documentation technique détaillée, incluant les références API et les détails d'implémentation, veuillez consulter la **documentation générée par Doxygen**, disponible sur la page GitLab Pages du projet.

Pour générer la documentation :

```bash
doxygen Doxyfile
```

Puis ouvrez `docs/html/index.html` dans votre navigateur.

## Prise en main

### Prérequis

- Compilateur GCC
- Make
- Doxygen (pour la documentation)

### Compilation

```bash
make build
```

### Tests

```bash
make test
```

## Auteurs

- Mattéo BEZET-TORRES
- Elias GARACH-MALULY
- Anton MOULIN
