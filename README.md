# minishell

[Français](#francais) | [English](#english)

---

<a id="francais"></a>

## Français

Un shell inspiré de bash écrit en C pour le cursus 42, appuyé par une
bibliothèque d'utilitaires maison (`libft`).

L'objectif n'est pas de réimplémenter bash, mais de construire un shell petit et
lisible qui couvre le comportement de base attendu par le projet `minishell` de
42 : parsing, guillemets, expansions, builtins, pipes, redirections et gestion
des processus.

### Points clés

- Invite interactive basée sur GNU Readline avec historique des commandes.
- Parseur orienté tokens, façon objet : chaque token porte les fonctions
  utilisées pour se créer, se mettre à jour, s'afficher, s'ajouter, se détruire
  et s'exécuter.
- Environnement stocké sous forme de paires clé/valeur et exposé via les
  builtins du shell.
- Guillemets simples et doubles, y compris la détection des guillemets non
  fermés.
- Expansion de `$VAR`, `$?` et `$$`.
- Pipelines et redirections usuelles : `<`, `>`, `>>` et `<<` (heredoc).
- Commandes externes résolues via `PATH`, plus les principaux builtins.
- Les codes de sortie sont propagés entre les commandes et dans `$?`.
- Une `libft` réutilisable avec chaînes, vecteurs, listes, maps, optionnels,
  parsing d'arguments et tests.

### Non implémenté

- Opérateurs logiques `&&` et `||`.
- Expansion des jokers / glob (`*`, `?`, `[...]`).
- Compatibilité bash complète. C'est un shell pédagogique, pas un remplacement
  direct de bash.

### Compilation et exécution

#### Prérequis

- `clang`
- `make`
- En-têtes et bibliothèque de développement GNU Readline

Sous Debian/Ubuntu :

```sh
sudo apt install clang make libreadline-dev
```

#### Compilation

```sh
make
./minishell
```

`make` compile d'abord `libft`, puis le shell dans `./minishell`. Les objets sont
écrits dans `build/`, et les erreurs de compilation sont journalisées dans
`compilation.log`.

Avec Nix, l'environnement fourni apporte Readline, `pkg-config` et Valgrind :

```sh
nix-shell --run make
./minishell
```

#### Cibles Make

| Cible         | Description                                        |
| ------------- | -------------------------------------------------- |
| `all`         | Compile `./minishell` (par défaut)                 |
| `clean`       | Supprime les objets de compilation et les logs     |
| `fclean`      | Supprime aussi le binaire et nettoie `libft`       |
| `re`          | `fclean` puis `all`                                |
| `tests_run`   | Lance la suite de tests de `libft`                 |

### Utilisation

```sh
minishell$> echo "hello from minishell"
hello from minishell

minishell$> export GREETING=hi
minishell$> echo "$GREETING $USER"

minishell$> ls -la | grep minishell > files.txt
minishell$> cat << EOF
> some heredoc content
> EOF
```

Builtins pris en charge :

| Builtin  | Notes                                              |
| -------- | -------------------------------------------------- |
| `echo`   | Prend en charge `-n`                               |
| `cd`     | Prend en charge `~`, `-`, `--`, `HOME`, `OLDPWD`, `PWD` |
| `pwd`    | Affiche le répertoire de travail courant           |
| `env`    | Affiche l'environnement                            |
| `export` | Définit des variables ; les affiche sans argument  |
| `unset`  | Supprime des variables                             |
| `exit`   | Gère les arguments numériques et la plage 0–255    |

### Architecture

Le shell est construit autour d'un pipeline de tokens. Chaque étape est petite
et globalement indépendante, ce qui garde le parseur extensible et sépare la
logique d'exécution de la logique syntaxique.

```
entrée readline
      |
      v
expansion brute      $VAR, $?, $$, contexte de guillemets
      |
      v
tokenizer            les validateurs sélectionnent le token suivant
      |
      v
nœuds préparés       table de fonctions par token (create/append/execute/...)
      |
      v
syntaxe              mots, pipes, redirections, fin de ligne
      |
      v
t_cmd_to_exec        argv, liste de redirections, chemin de la commande
      |
      v
exécuteur            builtin ou fork/execve, pipes, redirections, heredocs
```

#### Les tokens comme objets

L'idée centrale est que le comportement d'un token est attaché au token
lui-même. Un `t_preparsed_node` contient son type, sa valeur et des pointeurs
de fonction pour les opérations dont le parseur peut avoir besoin :

```c
struct s_preparsed_node
{
    t_tok_type       type;
    void            *value;
    t_tok_action     create;
    t_tok_action     ulb;
    t_tok_action     append;
    t_preparsed_apply print;
    t_preparsed_apply destroy;
    t_preparsed_exec execute;
};
```

`preparsed_node_factory()` cherche l'entrée correspondante dans une table
statique et installe les bonnes fonctions pour le type de token. Ajouter un
nouveau type de token revient à ajouter un nouvel ensemble de fonctions de
token et une entrée dans la table, plutôt qu'à ajouter des cas particuliers
partout dans le parseur.

Types de tokens actuellement gérés :

- `TOK_PIPE` — `|`
- `TOK_REDIR` — `<`, `>`, `>>`, `<<`
- `TOK_WORD` — noms de commandes et arguments
- `TOK_SPACE` — espaces entre les tokens
- `TOK_QUOTE` — chaînes entre guillemets simples et doubles
- `TOK_EOL` — fin de l'entrée
- `TOK_UNKNOWN` — entrée invalide ou inattendue

#### Exécution

Le parseur transforme les tokens préparés en valeurs `t_cmd_to_exec`.
L'exécuteur traite ensuite les commandes une par une, en connectant les pipes
et les redirections, en exécutant les builtins dans le processus du shell et en
forkant les commandes externes.

### Organisation du projet

```
include/                 En-têtes publics et types principaux
src/
  core/                  Invite, boucle, exécution, pipes, redirections,
                         heredoc, expansions, signaux, nettoyage
  parser/                Tokenizer, vérification syntaxique, construction
                         des commandes
  parser/tok_func/       Implémentations par token (pipe, redir, word, ...)
  builtins/              echo, cd, pwd, env, export, unset, exit, appels externes
  env/                   Helpers de stockage de l'environnement
  pair/                  Helpers de paires clé/valeur utilisés par l'environnement
  ft_addons/             Petits utilitaires au-dessus de libft
libft/                   Bibliothèque d'utilitaires maison et suite de tests
build/                   Fichiers objets générés
Makefile                 Orchestration de la compilation
shell.nix                Environnement de développement Nix
```

### libft

`libft` est une bibliothèque d'utilitaires étendue utilisée par le shell. Elle
est compilée en archive statique (`libft/libft_personal.a`) et contient :

- `ft_string` : helpers pour caractères, mémoire et chaînes.
- `t_string` : type de chaîne dynamique.
- `ft_vector` : tableau dynamique avec helpers map/filter/sort.
- `ft_list` : listes simplement et doublement chaînées.
- `ft_map` : table de hachage.
- `ft_optional` : valeurs optionnelles avec helpers de chaînage.
- `ft_math`, `ft_args`, `get_next_line` et divers helpers internes.

Ses tests se trouvent dans `libft/tests/` et se lancent avec :

```sh
make tests_run
```

### Notes et limites

- Les guillemets non fermés sont détectés et signalés comme une erreur au lieu
  de faire planter le shell.
- Le shell est un projet scolaire 42 et vise le comportement demandé par le
  sujet, plus quelques ajouts pratiques. Il laisse volontairement de côté des
  fonctionnalités bash plus avancées comme les opérateurs logiques et le
  globbing.
- Le parseur conserve une liste de types de tokens potentiellement futurs (par
  exemple les points-virgules, `&&`, `||` et la substitution de commande) dans
  `include/parser_types.h`.

### Auteurs

Écrit par `bgoulard` et `nrobinso` dans le cadre du cursus 42.

---

<a id="english"></a>

## English

A bash-like shell written in C for the 42 curriculum, backed by a custom
utility library (`libft`).

The goal is not to reimplement bash, but to build a small, readable shell that
covers the core behaviour expected by the 42 `minishell` project: parsing,
quotes, expansions, builtins, pipes, redirections and process management.

### Highlights

- Interactive prompt based on GNU Readline with command history.
- Token-oriented, object-like parser: every token carries the functions used to
  create, update, print, append, destroy and execute itself.
- Environment stored as key/value pairs and exposed through shell builtins.
- Single and double quotes, including unterminated-quote detection.
- `$VAR`, `$?` and `$$` expansion.
- Pipelines and the usual redirections: `<`, `>`, `>>` and `<<` (heredoc).
- External commands resolved through `PATH`, plus the main builtins.
- Exit codes are propagated between commands and into `$?`.
- A reusable `libft` with strings, vectors, lists, maps, optionals, argument
  parsing and tests.

### Not implemented

- `&&` and `||` logical operators.
- Wildcard / glob expansion (`*`, `?`, `[...]`).
- Full bash compatibility. This is a pedagogical shell, not a drop-in bash
  replacement.

### Build and run

#### Requirements

- `clang`
- `make`
- GNU Readline development headers and library

On Debian/Ubuntu:

```sh
sudo apt install clang make libreadline-dev
```

#### Build

```sh
make
./minishell
```

`make` builds `libft` first, then compiles the shell into `./minishell`.
Objects are written to `build/`, and compilation errors are logged to
`compilation.log`.

With Nix, the provided shell environment brings in Readline, `pkg-config` and
Valgrind:

```sh
nix-shell --run make
./minishell
```

#### Make targets

| Target        | Description                                       |
| ------------- | ------------------------------------------------- |
| `all`         | Build `./minishell` (default)                     |
| `clean`       | Remove build objects and logs                     |
| `fclean`      | Also remove the binary and clean `libft`          |
| `re`          | `fclean` followed by `all`                        |
| `tests_run`   | Run the `libft` test suite                        |

### Usage

```sh
minishell$> echo "hello from minishell"
hello from minishell

minishell$> export GREETING=hi
minishell$> echo "$GREETING $USER"

minishell$> ls -la | grep minishell > files.txt
minishell$> cat << EOF
> some heredoc content
> EOF
```

Supported builtins:

| Builtin  | Notes                                            |
| -------- | ------------------------------------------------ |
| `echo`   | Supports `-n`                                    |
| `cd`     | Supports `~`, `-`, `--`, `HOME`, `OLDPWD`, `PWD` |
| `pwd`    | Prints the current working directory             |
| `env`    | Prints the environment                           |
| `export` | Sets variables; prints them without arguments    |
| `unset`  | Removes variables                                |
| `exit`   | Handles numeric arguments and the 0–255 range    |

### Architecture

The shell is built around a token pipeline. Each stage is small and mostly
independent, which keeps the parser extensible and the execution logic
separate from the syntax logic.

```
readline input
     |
     v
raw expansion        $VAR, $?, $$, quote context
     |
     v
tokenizer            validators select the next token
     |
     v
preparsed nodes      per-token function table (create/append/execute/...)
     |
     v
syntax check         words, pipes, redirections, end of line
     |
     v
t_cmd_to_exec        argv, redirection list, command path
     |
     v
executor             builtin or fork/execve, pipes, redirections, heredocs
```

#### Tokens as objects

The central idea is that token behaviour is attached to the token itself. A
`t_preparsed_node` holds its type, its value, and function pointers for the
operations the parser may need:

```c
struct s_preparsed_node
{
    t_tok_type       type;
    void            *value;
    t_tok_action     create;
    t_tok_action     ulb;
    t_tok_action     append;
    t_preparsed_apply print;
    t_preparsed_apply destroy;
    t_preparsed_exec execute;
};
```

`preparsed_node_factory()` looks up the matching entry in a static table and
installs the right functions for the token type. Adding a new token kind means
adding a new set of token functions and one table entry, rather than adding
special cases throughout the parser.

Token types currently handled:

- `TOK_PIPE` — `|`
- `TOK_REDIR` — `<`, `>`, `>>`, `<<`
- `TOK_WORD` — command names and arguments
- `TOK_SPACE` — whitespace between tokens
- `TOK_QUOTE` — single and double quoted strings
- `TOK_EOL` — end of input
- `TOK_UNKNOWN` — invalid or unexpected input

#### Execution

The parser turns the preparsed tokens into `t_cmd_to_exec` values. The executor
then processes them one command at a time, wiring pipes and redirections,
running builtins in the shell process, and forking external commands.

### Project layout

```
include/                 Public headers and core types
src/
  core/                  Prompt, loop, execution, pipes, redirections,
                         heredoc, expansions, signals, cleanup
  parser/                Tokenizer, syntax checking, command construction
  parser/tok_func/       Per-token implementations (pipe, redir, word, ...)
  builtins/              echo, cd, pwd, env, export, unset, exit, external calls
  env/                   Environment storage helpers
  pair/                  Key/value pair helpers used by the environment
  ft_addons/             Small utilities layered on top of libft
libft/                   Custom utility library and test suite
build/                   Generated object files
Makefile                 Build orchestration
shell.nix                Nix development environment
```

### libft

`libft` is an extended utility library used by the shell. It is built as a
static archive (`libft/libft_personal.a`) and includes:

- `ft_string`: character, memory and string helpers.
- `t_string`: growable string type.
- `ft_vector`: dynamic array with map/filter/sort helpers.
- `ft_list`: singly and doubly linked lists.
- `ft_map`: hash map.
- `ft_optional`: optional values with chaining helpers.
- `ft_math`, `ft_args`, `get_next_line` and various internal helpers.

Its tests live in `libft/tests/` and are run with:

```sh
make tests_run
```

### Notes and limitations

- Unterminated quotes are detected and reported as an error instead of
  crashing the shell.
- The shell is a 42 school project and targets the behaviour required by the
  subject plus a few practical additions. It intentionally leaves out more
  advanced bash features such as logical operators and globbing.
- The parser keeps a list of possible future token types (for example
  semicolons, `&&`, `||` and command substitution) in `include/parser_types.h`.

### Authors

Written by `bgoulard` and `nrobinso` as part of the 42 curriculum.
