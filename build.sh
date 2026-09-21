#!/usr/bin/env bash
set -e

echo "==================================="
echo "   CIAB - Compilation (Linux/Unix)"
echo "==================================="

# 1. Verification de la presence de CMake
if ! command -v cmake &> /dev/null; then
    echo ""
    echo "[ERREUR] CMake n'est pas installe ou n'est pas dans le PATH."
    exit 1
fi

# 2. Configuration avec CMake
echo "[1/3] Configuration avec CMake..."
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release

# 3. Compilation du projet
echo ""
echo "[2/3] Compilation du projet..."
cmake --build build --config Release

# 4. Execution des tests via CTest
echo ""
echo "[3/3] Execution des tests..."
ctest --test-dir build --output-on-failure -C Release --verbose

echo ""
echo "==================================="
echo "[SUCCES] Compilation et tests valides avec succes !"
echo "==================================="
