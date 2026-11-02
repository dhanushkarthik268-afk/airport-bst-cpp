// Airport.cpp - parsing and printing of airport records.
#include "Airport.h"

#include <cstdlib>
#include <iomanip>
#include <sstream>

Airport::Airport(const std::string& code, const std::string& name, const std::string& city,
                 const std::string& country, const std::string& icao,
                 double latitude, double longitude)
    : code(code), name(name), city(city), country(country), icao(icao),
      latitude(latitude), longitude(longitude) {}

std::string Airport::toString() const {
    return code + "  " + name + ", " + city + ", " + country;
}

std::string Airport::details() const {
    std::ostringstream out;
    out << toString() << "\n    ICAO: " << (icao.empty() ? "-" : icao)
        << "   lat " << std::fixed << std::setprecision(5) << latitude
        << ", lon " << longitude;
    return out.str();
}

std::vector<std::string> Airport::splitCsvLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;
    int n = static_cast<int>(line.size());
    for (int i = 0; i < n; i++) {
        char c = line[i];
        if (c == '\r') continue;                       // tolerate Windows line endings
        if (inQuotes) {
            if (c == '"') {
                if (i + 1 < n && line[i + 1] == '"') { // "" inside quotes -> one "
                    current += '"';
                    i++;
                } else {
                    inQuotes = false;                  // closing quote
                }
            } else {
                current += c;                          // commas inside quotes are kept
            }
        } else {
            if (c == '"') inQuotes = true;
            else if (c == ',') { fields.push_back(current); current.clear(); }
            else current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

static double toDouble(const std::string& s) {
    if (s.empty() || s == "\\N") return 0.0;
    return std::strtod(s.c_str(), nullptr);
}

Airport::ParseStatus Airport::parseOpenFlightsLine(const std::string& line, Airport& out) {
    std::vector<std::string> f = splitCsvLine(line);
    if (f.size() < 8) return ParseStatus::Malformed;
    if (f[4] == "\\N" || f[4].size() != 3) return ParseStatus::NoIata;
    out.name      = f[1];
    out.city      = f[2];
    out.country   = f[3];
    out.code      = f[4];
    out.icao      = (f[5] == "\\N") ? "" : f[5];
    out.latitude  = toDouble(f[6]);
    out.longitude = toDouble(f[7]);
    return ParseStatus::Ok;
}

std::string Airport::toRecordLine() const {
    std::ostringstream out;
    out << code << '\t' << name << '\t' << city << '\t' << country << '\t' << icao << '\t'
        << std::setprecision(12) << latitude << '\t' << longitude;
    return out.str();
}

bool Airport::fromRecordLine(const std::string& line, Airport& out) {
    std::vector<std::string> f;
    std::string current;
    int n = static_cast<int>(line.size());
    for (int i = 0; i < n; i++) {
        if (line[i] == '\r') continue;
        if (line[i] == '\t') { f.push_back(current); current.clear(); }
        else current += line[i];
    }
    f.push_back(current);
    if (f.empty() || f[0].empty()) return false;
    while (f.size() < 7) f.push_back("");
    out.code      = f[0];
    out.name      = f[1];
    out.city      = f[2];
    out.country   = f[3];
    out.icao      = f[4];
    out.latitude  = toDouble(f[5]);
    out.longitude = toDouble(f[6]);
    return true;
}
