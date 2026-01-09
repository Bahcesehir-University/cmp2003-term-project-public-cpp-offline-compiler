#include "analyzer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype>

using namespace std;

// ===================== Helper Implementations =====================

string TripAnalyzer::trimCopy(const string& s) {
    if (s.empty()) return s;
    size_t i = 0, j = s.size();
    while (i < j && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) i++;
    while (j > i && (s[j - 1] == ' ' || s[j - 1] == '\t' || s[j - 1] == '\r' || s[j - 1] == '\n')) j--;
    return s.substr(i, j - i);
}

bool TripAnalyzer::splitComma6(const string& line, string out[6]) {
    int idx = 0;
    size_t start = 0;
    
    while (idx < 6) {
        size_t pos = line.find(',', start);
        if (pos == string::npos) {
            out[idx++] = line.substr(start);
            break; 
        }
        out[idx++] = line.substr(start, pos - start);
        start = pos + 1;
    }
    
    // Ensure we parsed exactly 6 columns
    if (idx != 6) return false;
    
    // Trim whitespace from all columns
    for (int i = 0; i < 6; i++) out[i] = trimCopy(out[i]);
    return true;
}

bool TripAnalyzer::parseHour(const string& dt, int& hour) {
    string s = trimCopy(dt);
    size_t colon = s.find(':');
    if (colon == string::npos) return false;
    if (colon == 0) return false;

    // Parse backwards from the colon to find the hour digits
    size_t p = colon;
    // Skip trailing spaces before colon if any
    while (p > 0 && (s[p - 1] == ' ' || s[p - 1] == '\t')) p--;
    
    size_t end = p;
    // Find start of digits
    while (p > 0 && isdigit(static_cast<unsigned char>(s[p - 1]))) p--;
    
    if (p == end) return false; // No digits found

    string hs = s.substr(p, end - p);
    if (hs.size() > 2) return false;

    int h = 0;
    for (char c : hs) {
        if (!isdigit(static_cast<unsigned char>(c))) return false;
        h = h * 10 + (c - '0');
    }
    
    if (h < 0 || h > 23) return false;
    hour = h;
    return true;
}

// ===================== Public Method Implementations =====================

void TripAnalyzer::ingestFile(const std::string& filepath) {
    // Reset data for fresh ingestion
    zoneCounts.clear();
    slotCounts.clear();

    std::ifstream file(filepath);
    
    // If file cannot be opened, return immediately (robustness check)
    if (!file.is_open()) {
        return;
    }

    string line;
    bool first = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        // Skip header row
        if (first) {
            first = false;
            if (line.find("TripID") != string::npos && line.find("PickupZoneID") != string::npos) {
                continue;
            }
        }

        string cols[6];
        // Skip malformed lines (incorrect column count)
        if (!splitComma6(line, cols)) continue;

        const string& pickupZone = cols[1];
        const string& pickupDT   = cols[3];

        // Skip rows with missing essential data
        if (pickupZone.empty() || pickupDT.empty()) continue;

        int hour = -1;
        // Skip rows with invalid date/time format
        if (!parseHour(pickupDT, hour)) continue;

        // Update counts
        zoneCounts[pickupZone]++;

        auto it = slotCounts.find(pickupZone);
        if (it == slotCounts.end()) {
            array<long long, 24> arr{};
            arr.fill(0);
            arr[hour] = 1;
            slotCounts.emplace(pickupZone, arr);
        } else {
            it->second[hour]++;
        }
    }
}

vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    if (k <= 0 || zoneCounts.empty()) return {};

    vector<ZoneCount> all;
    all.reserve(zoneCounts.size());
    for(const auto& kv : zoneCounts) {
        all.push_back({kv.first, kv.second});
    }

    // Sort order: Count (Desc) -> Zone Name (Asc)
    std::sort(all.begin(), all.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if ((int)all.size() > k) {
        all.resize(k);
    }
    return all;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    if (k <= 0 || slotCounts.empty()) return {};

    vector<SlotCount> all;
    // Flatten map to vector
    for (const auto& kv : slotCounts) {
        const string& zone = kv.first;
        const auto& arr = kv.second;
        for (int h = 0; h < 24; h++) {
            if (arr[h] > 0) {
                all.push_back({zone, h, arr[h]});
            }
        }
    }

    // Sort order: Count (Desc) -> Zone Name (Asc) -> Hour (Asc)
    std::sort(all.begin(), all.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if ((int)all.size() > k) {
        all.resize(k);
    }
    return all;
}
