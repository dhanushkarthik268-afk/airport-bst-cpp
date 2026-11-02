// Airport.h - one airport record from the OpenFlights database.
#ifndef AIRPORT_H
#define AIRPORT_H

#include <string>
#include <vector>

class Airport {
public:
    std::string code;      // IATA code, the BST key (e.g. "BOS")
    std::string name;      // e.g. "General Edward Lawrence Logan International Airport"
    std::string city;      // e.g. "Boston"
    std::string country;   // e.g. "United States"
    std::string icao;      // e.g. "KBOS" (empty if missing)
    double latitude = 0.0;
    double longitude = 0.0;

    Airport() = default;
    Airport(const std::string& code, const std::string& name, const std::string& city,
            const std::string& country, const std::string& icao = "",
            double latitude = 0.0, double longitude = 0.0);

    // One-line summary: "BOS  Name, City, Country"
    std::string toString() const;
    // Summary plus ICAO code and coordinates.
    std::string details() const;

    // Result of parsing one line of airports.dat.
    enum class ParseStatus { Ok, NoIata, Malformed };

    // Parses one OpenFlights line into `out`. Returns NoIata when field 5 is \N.
    static ParseStatus parseOpenFlightsLine(const std::string& line, Airport& out);

    // Splits a CSV line on commas that are outside double quotes.
    // A doubled quote ("") inside a quoted field becomes one quote character.
    static std::vector<std::string> splitCsvLine(const std::string& line);

    // Tab-separated form used by `save` / `load` (preorder files).
    std::string toRecordLine() const;
    static bool fromRecordLine(const std::string& line, Airport& out);
};

#endif
