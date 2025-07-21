#!/bin/bash

# 6502 Simulator Development Utility Script
# This script provides convenient commands for common development tasks

set -e  # Exit on any error

PROJECT_NAME="6502 Simulator"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Function to print colored output
print_colored() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    echo
    print_colored $BLUE "=============================================="
    print_colored $BLUE "🔧 $PROJECT_NAME - Development Utility"
    print_colored $BLUE "=============================================="
    echo
}

print_help() {
    print_header
    echo "Available commands:"
    echo
    print_colored $GREEN "Build Commands:"
    echo "  ./dev.sh build      - Build the simulator"
    echo "  ./dev.sh clean      - Clean build artifacts"
    echo "  ./dev.sh rebuild    - Clean and rebuild"
    echo "  ./dev.sh debug      - Build with debug symbols"
    echo
    print_colored $GREEN "Test Commands:"
    echo "  ./dev.sh test       - Run all 75 functional tests"
    echo "  ./dev.sh test-quick - Run quick test suite"
    echo "  ./dev.sh coverage   - Generate code coverage report"
    echo
    print_colored $GREEN "Development Commands:"
    echo "  ./dev.sh format     - Format source code"
    echo "  ./dev.sh lint       - Run static analysis"
    echo "  ./dev.sh install    - Install to system"
    echo "  ./dev.sh package    - Create distribution package"
    echo
    print_colored $GREEN "Utility Commands:"
    echo "  ./dev.sh info       - Show project information"
    echo "  ./dev.sh help       - Show this help"
    echo
}

cmd_build() {
    print_colored $CYAN "🔨 Building $PROJECT_NAME..."
    make all
    print_colored $GREEN "✅ Build completed!"
}

cmd_clean() {
    print_colored $CYAN "🧹 Cleaning build artifacts..."
    make clean
    print_colored $GREEN "✅ Clean completed!"
}

cmd_rebuild() {
    print_colored $CYAN "🔄 Rebuilding $PROJECT_NAME..."
    make rebuild
    print_colored $GREEN "✅ Rebuild completed!"
}

cmd_debug() {
    print_colored $CYAN "🐛 Building debug version..."
    make debug
    print_colored $GREEN "✅ Debug build completed!"
}

cmd_test() {
    print_colored $CYAN "🧪 Running full test suite..."
    make test
    print_colored $GREEN "✅ All tests completed!"
}

cmd_test_quick() {
    print_colored $CYAN "⚡ Running quick tests..."
    make test-quick
    print_colored $GREEN "✅ Quick tests completed!"
}

cmd_coverage() {
    print_colored $CYAN "📊 Generating coverage report..."
    if command -v gcov >/dev/null 2>&1; then
        # Add coverage flags and rebuild
        CFLAGS="-fprofile-arcs -ftest-coverage" LDFLAGS="-lgcov --coverage" make rebuild
        make test-quick >/dev/null 2>&1 || true

        # Generate coverage report
        gcov src/*.c
        print_colored $GREEN "✅ Coverage files generated (*.gcov)"
    else
        print_colored $YELLOW "⚠️  gcov not found. Install gcc with coverage support."
    fi
}

cmd_format() {
    print_colored $CYAN "🎨 Formatting source code..."
    if command -v clang-format >/dev/null 2>&1; then
        find src tests -name "*.c" -o -name "*.h" | xargs clang-format -i
        print_colored $GREEN "✅ Code formatted!"
    else
        print_colored $YELLOW "⚠️  clang-format not found. Skipping format."
    fi
}

cmd_lint() {
    print_colored $CYAN "🔍 Running static analysis..."
    if command -v cppcheck >/dev/null 2>&1; then
        cppcheck --enable=all --suppress=missingIncludeSystem src/ tests/
        print_colored $GREEN "✅ Static analysis completed!"
    else
        print_colored $YELLOW "⚠️  cppcheck not found. Install for static analysis."
    fi
}

cmd_install() {
    print_colored $CYAN "📦 Installing to system..."
    make install
    print_colored $GREEN "✅ Installation completed!"
}

cmd_package() {
    print_colored $CYAN "📦 Creating distribution package..."
    VERSION=$(grep -o 'v[0-9]*\.[0-9]*\.[0-9]*' README.md 2>/dev/null || echo "v1.0.0")
    PACKAGE_NAME="sim65-${VERSION}"

    # Create temporary directory
    rm -rf dist
    mkdir -p "dist/${PACKAGE_NAME}"

    # Copy files
    cp -r src tests asm Makefile README.md LICENSE* "dist/${PACKAGE_NAME}/" 2>/dev/null || true

    # Create tarball
    cd dist
    tar -czf "${PACKAGE_NAME}.tar.gz" "${PACKAGE_NAME}"
    cd ..

    print_colored $GREEN "✅ Package created: dist/${PACKAGE_NAME}.tar.gz"
}

cmd_info() {
    make info
}

# Main command dispatcher
case "${1:-help}" in
    build)
        cmd_build
        ;;
    clean)
        cmd_clean
        ;;
    rebuild)
        cmd_rebuild
        ;;
    debug)
        cmd_debug
        ;;
    test)
        cmd_test
        ;;
    test-quick)
        cmd_test_quick
        ;;
    coverage)
        cmd_coverage
        ;;
    format)
        cmd_format
        ;;
    lint)
        cmd_lint
        ;;
    install)
        cmd_install
        ;;
    package)
        cmd_package
        ;;
    info)
        cmd_info
        ;;
    help|--help|-h)
        print_help
        ;;
    *)
        print_colored $RED "❌ Unknown command: $1"
        echo
        print_help
        exit 1
        ;;
esac
