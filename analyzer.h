#ifndef TRIP_ANALYZER_H
#define TRIP_ANALYZER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <algorithm>

using namespace std;

struct ZoneCount {
    string zone;
    long long count;
};

struct SlotCount {
    string zone;
    int hour;
    long long count;
};

class TripAnalyzer {
public:
    void ingestFile(const std::string& filepath);
    vector<ZoneCount> topZones(int k = 10) const;
    vector<SlotCount> topBusySlots(int k = 10) const;

private:
    unordered_map<string, long long> zoneCounts;
    unordered_map<string, array<long long, 24>> slotCounts;

    // Helper functions
    static string trimCopy(const string& s);
    static bool splitComma6(const string& line, string out[6]);
    static bool parseHour(const string& dt, int& hour);
};

#endif
