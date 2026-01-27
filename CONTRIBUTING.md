# Contributing to [Project Name]

First off, thank you for considering contributing! 

## Development Environment Setup
To get a fully working environment with autocomplete:
1. Install **Qt6**, **make**, and **Bear**.
2. Run `qmake6 project.pro`.
3. Run `make lsp` to generate the `compile_commands.json`.

## Code Style
Please run `make format` (or use clang-format) before submitting a PR.

## Old installation stuff

1. **Only Qt 6 is supported.**
2. Install all Qt 6 packages (e.g., `qt6-base`, `qt6-multimedia`, etc.).
3. Use `qmake6` to generate Makefiles.
4. Run `make` to build the project.
5. Execute with `./game_menu`.
