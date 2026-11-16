{ pkgs ? import <nixpkgs> { } }:

# Development shell for minishell.
# Provides the GNU readline headers/libs required by the Makefile
# (`-lreadline`, <readline/readline.h>, <readline/history.h>).
#
# Usage:
#   nix-shell --run make
#   nix-shell          # then run `make` manually
pkgs.mkShell {
  packages = [
    pkgs.readline
    pkgs.pkg-config
    pkgs.valgrind
  ];

  hardeningDisable = [ "all" ];
}
