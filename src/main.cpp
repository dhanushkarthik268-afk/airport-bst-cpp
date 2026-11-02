// main.cpp - command-line airport lookup service built on AirportBST.
//
// Build:  g++ -std=c++17 -O2 src/*.cpp -o airports
// Run:    ./airports                 (interactive)
//         ./airports --echo < script (echoes each command, for recorded sessions)
//
// Data: OpenFlights.org airport database (Open Database License).
#include "Airport.h"
#include "AirportBST.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

static const std::string kSaveHeader = "# airport-bst preorder v1";

// ------------------------------------------------------------------ session state
struct Session {
    std::vector<Airport> records;   // airports.dat records in file order (the master list)
    AirportBST tree;
    bool echo = false;
};

// ------------------------------------------------------------------ small helpers
static std::string upper(std::string s) {
    int n = static_cast<int>(s.size());
    for (int i = 0; i < n; i++)
        if (s[i] >= 'a' && s[i] <= 'z') s[i] = static_cast<char>(s[i] - 'a' + 'A');
    return s;
}

static std::string trim(const std::string& s) {
    size_t b = s.find_first_not_of(" \t\r");
    if (b == std::string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r");
    return s.substr(b, e - b + 1);
}

static std::string fmt1(double x) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(1) << x;
    return out.str();
}

static void printShape(const AirportBST& tree) {
    std::cout << "Height: " << tree.height() << "  Average depth: " << fmt1(tree.averageDepth())
              << "  Nodes: " << tree.size() << "\n";
}

static void printTopLevels(const AirportBST& tree, int levels, const std::string& label) {
    std::vector<std::vector<std::string>> lv = tree.levelOrder(levels);
    std::cout << "  " << label << ":";
    int n = static_cast<int>(lv.size());
    for (int i = 0; i < n; i++) {
        std::cout << "  L" << i << " [";
        int m = static_cast<int>(lv[i].size());
        for (int j = 0; j < m; j++) std::cout << (j ? " " : "") << lv[i][j];
        std::cout << "]";
    }
    std::cout << "\n";
}

static const char* okText(bool ok) { return ok ? "OK" : "FAILED"; }

// Insert records in the given order; optionally validate after every insert.
static void buildFrom(AirportBST& tree, const std::vector<Airport>& order, bool checkEach,
                      int* duplicates, std::vector<std::string>* dupCodes, bool* allValid) {
    tree.clear();
    int dups = 0;
    bool valid = true;
    int n = static_cast<int>(order.size());
    for (int i = 0; i < n; i++) {
        if (!tree.insert(order[i])) {
            dups++;
            if (dupCodes) dupCodes->push_back(order[i].code);
        }
        if (checkEach && !tree.validate()) valid = false;
    }
    if (duplicates) *duplicates = dups;
    if (allValid) *allValid = valid;
}

static std::vector<Airport> sortedCopy(const std::vector<Airport>& records) {
    std::vector<Airport> sorted = records;
    std::sort(sorted.begin(), sorted.end(),
              [](const Airport& a, const Airport& b) { return a.code < b.code; });
    return sorted;
}

// ------------------------------------------------------------------ Task 1: load
static bool loadOpenFlights(Session& s, const std::string& path, std::ifstream& in,
                            const std::string& firstLine) {
    s.records.clear();
    int totalLines = 0, skipped = 0, malformed = 0;
    std::string line = firstLine;
    bool haveLine = true;
    while (haveLine) {
        if (!line.empty()) {
            totalLines++;
            Airport a;
            Airport::ParseStatus st = Airport::parseOpenFlightsLine(line, a);
            if (st == Airport::ParseStatus::Ok) s.records.push_back(a);
            else if (st == Airport::ParseStatus::NoIata) skipped++;
            else malformed++;
        }
        haveLine = static_cast<bool>(std::getline(in, line));
    }

    int dups = 0;
    std::vector<std::string> dupCodes;
    buildFrom(s.tree, s.records, false, &dups, &dupCodes, nullptr);

    std::cout << "Loaded " << s.tree.size() << " airports (" << skipped
              << " lines skipped: no IATA code). Duplicates: " << dups << "\n";
    if (malformed) std::cout << "Warning: " << malformed << " malformed lines ignored\n";
    int nd = static_cast<int>(dupCodes.size());
    for (int i = 0; i < nd; i++) std::cout << "  duplicate (rule I4): " << dupCodes[i] << "\n";
    std::cout << "Lines read: " << totalLines << "  Records stored: " << s.records.size()
              << "  from " << path << "\n";
    std::cout << "Height: " << s.tree.height() << "  Average depth: "
              << fmt1(s.tree.averageDepth()) << "\n";
    return true;
}

static bool loadPreorder(Session& s, std::ifstream& in) {
    s.tree.clear();
    std::string line;
    int bad = 0, dups = 0;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        Airport a;
        if (!Airport::fromRecordLine(line, a)) { bad++; continue; }
        if (!s.tree.insert(a)) dups++;       // inserting in preorder rebuilds the same shape
    }
    std::cout << "Rebuilt tree from preorder file: " << s.tree.size() << " airports";
    if (bad) std::cout << ", " << bad << " bad lines";
    if (dups) std::cout << ", " << dups << " duplicates";
    std::cout << "\n";
    printShape(s.tree);
    return true;
}

static void cmdLoad(Session& s, const std::string& path) {
    std::ifstream in(path);
    if (!in) { std::cout << "Cannot open " << path << "\n"; return; }
    std::string first;
    if (!std::getline(in, first)) { std::cout << "File is empty\n"; return; }
    if (first.rfind(kSaveHeader, 0) == 0) loadPreorder(s, in);
    else loadOpenFlights(s, path, in, first);
}

// ------------------------------------------------------------------ Task 2: search
static void cmdFind(const Session& s, const std::string& codeIn) {
    std::string code = upper(codeIn);
    std::vector<std::string> path;
    int comps = 0;
    const Airport* a = s.tree.find(code, &path, &comps);
    std::cout << "Path: ";
    int n = static_cast<int>(path.size());
    for (int i = 0; i < n; i++) std::cout << (i ? " -> " : "") << path[i];
    if (a) {
        std::cout << " (comparisons: " << comps << ")\n" << a->details() << "\n";
    } else {
        std::cout << (n ? " -> " : "") << "(NULL) not found (comparisons: " << comps << ")\n";
    }
}

static void cmdMinMax(const Session& s) {
    const Airport* lo = s.tree.minimum();
    const Airport* hi = s.tree.maximum();
    if (!lo) { std::cout << "Tree is empty\n"; return; }
    std::cout << "Minimum: " << lo->toString() << "\n";
    std::cout << "Maximum: " << hi->toString() << "\n";
}

// ------------------------------------------------------------------ Task 3: delete / add
static void cmdRemove(Session& s, const std::string& codeIn) {
    std::string code = upper(codeIn);
    std::string detail;
    AirportBST::DeleteCase c = s.tree.remove(code, &detail);
    if (c == AirportBST::DeleteCase::NotFound) {
        std::cout << code << " not found, nothing deleted\n";
        return;
    }
    std::cout << "Deleted " << code << " (" << detail << ")  validate: "
              << okText(s.tree.validate()) << "\n";
}

// add CODE | Name | City | Country [| ICAO | lat | lon]
static void cmdAdd(Session& s, const std::string& rest) {
    std::vector<std::string> parts;
    std::string cur;
    int n = static_cast<int>(rest.size());
    for (int i = 0; i < n; i++) {
        if (rest[i] == '|') { parts.push_back(trim(cur)); cur.clear(); }
        else cur += rest[i];
    }
    parts.push_back(trim(cur));
    if (parts.size() < 4 || parts[0].size() != 3) {
        std::cout << "Usage: add CODE | Name | City | Country [| ICAO | lat | lon]\n";
        return;
    }
    Airport a(upper(parts[0]), parts[1], parts[2], parts[3]);
    if (parts.size() > 4) a.icao = parts[4];
    if (parts.size() > 5) a.latitude = std::atof(parts[5].c_str());
    if (parts.size() > 6) a.longitude = std::atof(parts[6].c_str());
    if (s.tree.insert(a))
        std::cout << "Added " << a.toString() << "  validate: " << okText(s.tree.validate()) << "\n";
    else
        std::cout << "Duplicate code " << a.code << " (rule I4): not inserted\n";
}

// ------------------------------------------------------------------ Task 4: traversals
static void cmdList(const Session& s, int limit) {
    std::vector<Airport> all = s.tree.inorder();
    int n = static_cast<int>(all.size());
    bool sorted = true;
    for (int i = 1; i < n; i++)
        if (!(all[i - 1].code < all[i].code)) sorted = false;
    int shown = (limit > 0 && limit < n) ? limit : n;
    for (int i = 0; i < shown; i++) std::cout << all[i].toString() << "\n";
    if (shown < n) std::cout << "... (" << (n - shown) << " more)\n";
    std::cout << n << " airports listed in inorder; sorted: " << (sorted ? "yes" : "NO") << "\n";
}

static void cmdSave(const Session& s, const std::string& path) {
    std::ofstream out(path);
    if (!out) { std::cout << "Cannot write " << path << "\n"; return; }
    std::vector<Airport> pre = s.tree.preorder();
    out << kSaveHeader << "\n";
    int n = static_cast<int>(pre.size());
    for (int i = 0; i < n; i++) out << pre[i].toRecordLine() << "\n";
    std::cout << "Saved " << n << " airports in preorder to " << path << "\n";
}

// save, reload into a second tree, compare height and full level order
static void cmdRoundTrip(const Session& s, const std::string& path) {
    cmdSave(s, path);
    std::ifstream in(path);
    std::string line;
    std::getline(in, line);  // header
    AirportBST copy;
    while (std::getline(in, line)) {
        Airport a;
        if (Airport::fromRecordLine(line, a)) copy.insert(a);
    }
    bool sameHeight = copy.height() == s.tree.height();
    bool sameLevels = copy.levelOrder() == s.tree.levelOrder();
    std::cout << "Original: height " << s.tree.height() << ", " << s.tree.size() << " nodes\n";
    std::cout << "Reloaded: height " << copy.height() << ", " << copy.size() << " nodes\n";
    std::cout << "Same height: " << (sameHeight ? "yes" : "NO")
              << "   Same level order (every level): " << (sameLevels ? "yes" : "NO")
              << "   validate: " << okText(copy.validate()) << "\n";
    printTopLevels(s.tree, 3, "original top 3 levels");
    printTopLevels(copy, 3, "reloaded top 3 levels");
}

static void cmdLevels(const Session& s, int levels) {
    std::vector<std::vector<std::string>> lv = s.tree.levelOrder(levels);
    int n = static_cast<int>(lv.size());
    for (int i = 0; i < n; i++) {
        std::cout << "L" << i << " (" << lv[i].size() << "):";
        int m = static_cast<int>(lv[i].size());
        for (int j = 0; j < m; j++) std::cout << " " << lv[i][j];
        std::cout << "\n";
    }
}

// ------------------------------------------------------------------ Task 5: range
static void cmdRange(const Session& s, const std::string& fromIn, const std::string& toIn,
                     bool quiet) {
    std::string from = upper(fromIn), to = upper(toIn);
    if (to < from) std::swap(from, to);
    std::vector<Airport> out;
    int visited = s.tree.range(from, to, out);
    int n = static_cast<int>(out.size());
    if (!quiet)
        for (int i = 0; i < n; i++) std::cout << out[i].toString() << "\n";
    std::cout << n << " airports, " << visited << " nodes visited\n";
}

// ------------------------------------------------------------------ statistics
static void cmdStats(const Session& s) {
    std::vector<std::string> countries;
    int us = 0;
    int n = static_cast<int>(s.records.size());
    for (int i = 0; i < n; i++) {
        countries.push_back(s.records[i].country);
        if (s.records[i].country == "United States") us++;
    }
    std::sort(countries.begin(), countries.end());
    countries.erase(std::unique(countries.begin(), countries.end()), countries.end());
    std::cout << "Records with an IATA code: " << n << "\n";
    std::cout << "Countries: " << countries.size() << "   Airports in the United States: " << us << "\n";
    printShape(s.tree);
}

// ------------------------------------------------------------------ Task 2 + 3 test script
static void cmdTest(Session& s) {
    if (s.records.empty()) { std::cout << "Load data first\n"; return; }
    std::cout << "=== Rebuilding in file order, validating after every insert ===\n";
    int dups = 0;
    bool allValid = true;
    buildFrom(s.tree, s.records, true, &dups, nullptr, &allValid);
    std::cout << "Inserted " << s.tree.size() << ", duplicates " << dups
              << ", validate after each of the " << s.records.size() << " inserts: "
              << okText(allValid) << "\n";
    printShape(s.tree);

    std::cout << "\n=== Task 2: search cases ===\n";
    std::string rootC = s.tree.rootCode();
    std::string leafC = s.tree.findCodeWithShape(AirportBST::Shape::Leaf);
    std::string deepC = s.tree.deepestCode();
    std::string minC = s.tree.minimum()->code;
    std::string maxC = s.tree.maximum()->code;
    std::string below = "000", above = "ZZZ";
    if (!(below < minC)) below = " ";     // fall back to something smaller than any code
    if (!(maxC < above)) above = "~~~";
    struct Case { const char* label; std::string code; };
    std::vector<Case> cases = {
        {"root", rootC}, {"a leaf (shallowest)", leafC}, {"deepest node", deepC},
        {"missing code", "XYZ"}, {"smaller than every key", below},
        {"larger than every key", above}};
    int nc = static_cast<int>(cases.size());
    for (int i = 0; i < nc; i++) {
        std::cout << "-- " << cases[i].label << ": find " << cases[i].code << "\n";
        cmdFind(s, cases[i].code);
    }
    cmdMinMax(s);

    std::cout << "\n=== Task 3: delete cases (top three levels before/after) ===\n";
    struct DelCase { const char* label; AirportBST::Shape shape; bool isRoot; };
    std::vector<DelCase> dels = {
        {"D1 leaf", AirportBST::Shape::Leaf, false},
        {"D2 only a left child", AirportBST::Shape::LeftOnly, false},
        {"D2 only a right child", AirportBST::Shape::RightOnly, false},
        {"D3 two children", AirportBST::Shape::TwoChildren, false},
        {"root", AirportBST::Shape::Leaf, true}};
    int nd = static_cast<int>(dels.size());
    for (int i = 0; i < nd; i++) {
        std::string code = dels[i].isRoot ? s.tree.rootCode()
                                          : s.tree.findCodeWithShape(dels[i].shape);
        std::cout << "-- " << dels[i].label << ": remove " << code << "\n";
        printTopLevels(s.tree, 3, "before");
        cmdRemove(s, code);
        printTopLevels(s.tree, 3, "after ");
        std::cout << "  find " << code << " afterwards: "
                  << (s.tree.contains(code) ? "still present (BUG)" : "not found (correct)") << "\n";
    }

    std::cout << "\n=== Task 3(c): made-up airport QQQ ===\n";
    cmdAdd(s, "QQQ | Dhan Ravichandran Field | Huntington | Imaginaria | XQQQ | 42.34 | -71.09");
    cmdFind(s, "QQQ");
    cmdRemove(s, "QQQ");
    cmdFind(s, "QQQ");

    std::cout << "\n=== Restoring the full tree in file order ===\n";
    buildFrom(s.tree, s.records, false, nullptr, nullptr, nullptr);
    printShape(s.tree);
}

// ------------------------------------------------------------------ Task 6: experiment
struct ExperimentRow {
    std::string label;
    int height;
    double avgDepth;
    double searchMs;
};

// Runs the 10,000 searches 5 times and reports the fastest run (less timer noise).
static double timeSearches(const AirportBST& tree, const std::vector<std::string>& queries) {
    double best = 1e18;
    int n = static_cast<int>(queries.size());
    for (int rep = 0; rep < 5; rep++) {
        auto t0 = std::chrono::steady_clock::now();
        long long found = 0;
        for (int i = 0; i < n; i++)
            if (tree.contains(queries[i])) found++;
        auto t1 = std::chrono::steady_clock::now();
        if (found != n) std::cout << "  (warning: only " << found << " of " << n << " found)\n";
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        if (ms < best) best = ms;
    }
    return best;
}

static ExperimentRow measure(const std::string& label, AirportBST& tree,
                             const std::vector<std::string>& queries) {
    ExperimentRow r;
    r.label = label;
    r.height = tree.height();
    r.avgDepth = tree.averageDepth();
    r.searchMs = timeSearches(tree, queries);
    return r;
}

static void cmdExperiment(Session& s) {
    if (s.records.empty()) { std::cout << "Load data first\n"; return; }
    int n = static_cast<int>(s.records.size());

    // the same 10,000 random (successful) searches for every tree
    std::mt19937 rng(2560);
    std::uniform_int_distribution<int> pick(0, n - 1);
    std::vector<std::string> queries;
    for (int i = 0; i < 10000; i++) queries.push_back(s.records[pick(rng)].code);

    std::vector<ExperimentRow> rows;
    AirportBST t;

    buildFrom(t, s.records, false, nullptr, nullptr, nullptr);
    rows.push_back(measure("File order", t, queries));

    std::vector<Airport> sorted = sortedCopy(s.records);
    buildFrom(t, sorted, false, nullptr, nullptr, nullptr);
    rows.push_back(measure("Sorted order", t, queries));

    t.buildBalanced(sorted);
    rows.push_back(measure("Balanced (middle rule)", t, queries));

    // bonus: several random shuffles
    std::vector<int> shuffleHeights;
    double sumDepth = 0, sumMs = 0;
    const int seeds = 10;
    for (int k = 0; k < seeds; k++) {
        std::vector<Airport> shuffled = s.records;
        std::mt19937 shuf(1000 + k);
        std::shuffle(shuffled.begin(), shuffled.end(), shuf);
        buildFrom(t, shuffled, false, nullptr, nullptr, nullptr);
        ExperimentRow r = measure("shuffle", t, queries);
        shuffleHeights.push_back(r.height);
        sumDepth += r.avgDepth;
        sumMs += r.searchMs;
    }
    int minH = *std::min_element(shuffleHeights.begin(), shuffleHeights.end());
    int maxH = *std::max_element(shuffleHeights.begin(), shuffleHeights.end());

    std::cout << "| Build order | Height | Average depth | Time for 10,000 searches |\n";
    std::cout << "|---|---|---|---|\n";
    int nr = static_cast<int>(rows.size());
    for (int i = 0; i < nr; i++)
        std::cout << "| " << rows[i].label << " | " << rows[i].height << " | "
                  << std::fixed << std::setprecision(2) << rows[i].avgDepth << " | "
                  << std::setprecision(3) << rows[i].searchMs << " ms |\n";
    std::cout << "| Random shuffle (" << seeds << " seeds) | " << minH << "-" << maxH
              << " | " << std::setprecision(2) << sumDepth / seeds << " (mean) | "
              << std::setprecision(3) << sumMs / seeds << " ms (mean) |\n";
    std::cout << std::defaultfloat;
    std::cout << "Shuffle heights:";
    for (int i = 0; i < seeds; i++) std::cout << " " << shuffleHeights[i];
    std::cout << "\n(The session tree was not changed.)\n";
}

static void cmdBuild(Session& s, const std::string& how, bool check) {
    if (s.records.empty()) { std::cout << "Load data first\n"; return; }
    std::vector<Airport> order;
    if (how == "file") order = s.records;
    else if (how == "sorted") order = sortedCopy(s.records);
    else if (how == "shuffle") {
        order = s.records;
        std::mt19937 rng(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::shuffle(order.begin(), order.end(), rng);
    } else {
        std::cout << "Usage: build file|sorted|shuffle [check]\n";
        return;
    }
    int dups = 0;
    bool valid = true;
    buildFrom(s.tree, order, check, &dups, nullptr, &valid);
    std::cout << "Built in " << how << " order: " << s.tree.size() << " airports, duplicates "
              << dups;
    if (check) std::cout << ", validate after every insert: " << okText(valid);
    std::cout << "\n";
    printShape(s.tree);
}

// ------------------------------------------------------------------ main loop
static void printHelp() {
    std::cout <<
        "Commands:\n"
        "  load FILE              load airports.dat (file order) or a saved preorder file\n"
        "  find CODE              search, printing the path and number of comparisons\n"
        "  min | max              smallest / largest code\n"
        "  remove CODE            delete an airport (reports case D1/D2/D3)\n"
        "  add CODE | Name | City | Country [| ICAO | lat | lon]\n"
        "  list [N]               inorder listing (optionally first N) and sortedness check\n"
        "  save FILE              write the tree in preorder\n"
        "  roundtrip FILE         save, reload into a new tree, compare shapes\n"
        "  validate               check the BST property with the (lo, hi) method\n"
        "  range FROM TO [-q]     airports with FROM <= code <= TO (-q: summary only)\n"
        "  levels [K]             level order (top K levels)\n"
        "  height | stats         tree shape / data facts\n"
        "  build file|sorted|shuffle [check]   rebuild from the loaded records\n"
        "  rebalance              balanced rebuild (middle element as root)\n"
        "  test                   run every Task 2 and Task 3 case\n"
        "  experiment             Task 6 shape experiment table\n"
        "  help | quit\n";
}

int main(int argc, char** argv) {
    Session s;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--echo") s.echo = true;

    std::cout << "Airport Code Lookup Service (BST). Type 'help' for commands.\n";
    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (s.echo) std::cout << line << "\n";
        if (line.empty() || line[0] == '#') continue;

        std::istringstream in(line);
        std::string cmd, a, b, c;
        in >> cmd;
        cmd = upper(cmd);
        in >> a >> b >> c;

        if (cmd == "QUIT" || cmd == "EXIT") break;
        else if (cmd == "HELP") printHelp();
        else if (cmd == "LOAD" && !a.empty()) cmdLoad(s, a);
        else if (cmd == "FIND" && !a.empty()) cmdFind(s, a);
        else if (cmd == "MIN" || cmd == "MAX") cmdMinMax(s);
        else if (cmd == "REMOVE" && !a.empty()) cmdRemove(s, a);
        else if (cmd == "ADD") cmdAdd(s, trim(line.substr(3)));
        else if (cmd == "LIST") cmdList(s, a.empty() ? 0 : std::atoi(a.c_str()));
        else if (cmd == "SAVE" && !a.empty()) cmdSave(s, a);
        else if (cmd == "ROUNDTRIP" && !a.empty()) cmdRoundTrip(s, a);
        else if (cmd == "VALIDATE") std::cout << "validate: " << okText(s.tree.validate()) << "\n";
        else if (cmd == "RANGE" && !b.empty()) cmdRange(s, a, b, c == "-q");
        else if (cmd == "LEVELS") cmdLevels(s, a.empty() ? 3 : std::atoi(a.c_str()));
        else if (cmd == "HEIGHT") printShape(s.tree);
        else if (cmd == "STATS") cmdStats(s);
        else if (cmd == "BUILD") cmdBuild(s, a, upper(b) == "CHECK");
        else if (cmd == "REBALANCE") { s.tree.rebalance(); printShape(s.tree); }
        else if (cmd == "TEST") cmdTest(s);
        else if (cmd == "EXPERIMENT") cmdExperiment(s);
        else std::cout << "Unknown command or missing argument. Type 'help'.\n";
    }
    return 0;
}
