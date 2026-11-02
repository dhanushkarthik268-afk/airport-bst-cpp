#!/bin/sh
# Downloads airports.dat from the OpenFlights repository (Open Database License).
# Prefer the copy posted on the course page; the repo version may differ slightly.
curl -L -o "$(dirname "$0")/airports.dat" \
  https://raw.githubusercontent.com/jpatokal/openflights/master/data/airports.dat
