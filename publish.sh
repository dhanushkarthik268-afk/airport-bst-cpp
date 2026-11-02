#!/bin/sh
# One-time publish to GitHub. First create an EMPTY public repo named airport-bst-cpp on github.com.
# Usage: ./publish.sh YOUR_GITHUB_USERNAME
set -e
if [ -z "$1" ]; then echo "Usage: ./publish.sh YOUR_GITHUB_USERNAME"; exit 1; fi
g++ -std=c++17 -O2 src/*.cpp -o airports
printf 'load data/airports.dat\nrange BOS BWI -q\nquit\n' | ./airports | grep -q "113 airports" && echo "Build check passed"
git init -q
git add .
git commit -q -m "Airport code lookup service with a binary search tree" || true
git branch -M main
git remote remove origin 2>/dev/null || true
git remote add origin "https://github.com/$1/airport-bst-cpp.git"
git push -u origin main
echo "Done: https://github.com/$1/airport-bst-cpp"
