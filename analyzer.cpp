#include "analyzer.h"
#include <iostream>

using namespace std;

// ===================== analyzer.cpp Implementation =====================

void TripAnalyzer::ingestStdin() {
    zoneCounts.clear();
    slotCounts.clear();

    string line;
    bool first = true;

    while (std::getline(cin, line)) {
        if (first) {
            first = false;
            if (line.find("TripID") != string::npos && line.find("PickupZoneID") != string::npos) continue;
        }
        if (line.empty()) continue;

        string cols[6];
        if (!splitComma6(line, cols)) continue;

        const string& pickupZone = cols[1];
        const string& pickupDT = cols[3];

        if (pickupZone.empty() || pickupDT.empty()) continue;

        int hour = -1;
        if (!parseHour(pickupDT, hour)) continue;

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

    struct Worse {
        bool operator()(const ZoneCount& a, const ZoneCount& b) const {
            if (a.count != b.count) return a.count > b.count;
            return a.zone < b.zone;
        }
    };

    priority_queue<ZoneCount, vector<ZoneCount>, Worse> pq;
    for (const auto& kv : zoneCounts) {
        ZoneCount z{kv.first, kv.second};
        if ((int)pq.size() < k) {
            pq.push(z);
        } else {
            const ZoneCount& w = pq.top();
            bool better = (z.count > w.count) || (z.count == w.count && z.zone < w.zone);
            if (better) {
                pq.pop();
                pq.push(z);
            }
        }
    }

    vector<ZoneCount> res;
    res.reserve(pq.size());
    while (!pq.empty()) {
        res.push_back(pq.top());
        pq.pop();
    }

    sort(res.begin(), res.end(), [](const ZoneCount& a, const ZoneCount& b) {
        if (a.count != b.count) return a.count > b.count;
        return a.zone < b.zone;
    });

    if ((int)res.size() > k) res.resize(k);
    return res;
}

vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    if (k <= 0 || slotCounts.empty()) return {};

    struct Worse {
        bool operator()(const SlotCount& a, const SlotCount& b) const {
            if (a.count != b.count) return a.count > b.count;
            if (a.zone != b.zone) return a.zone < b.zone;
            return a.hour < b.hour;
        }
    };

    priority_queue<SlotCount, vector<SlotCount>, Worse> pq;

    for (const auto& kv : slotCounts) {
        const string& zone = kv.first;
        const auto& arr = kv.second;
        for (int h = 0; h < 24; h++) {
            long long c = arr[h];
            if (c <= 0) continue;
            SlotCount s{zone, h, c};
            if ((int)pq.size() < k) {
                pq.push(s);
            } else {
                const SlotCount& w = pq.top();
                bool better = (s.count > w.count) ||
                              (s.count == w.count && (s.zone < w.zone ||
                               (s.zone == w.zone && s.hour < w.hour)));
                if (better) {
                    pq.pop();
                    pq.push(s);
                }
            }
        }
    }

    vector<SlotCount> res;
    res.reserve(pq.size());
    while (!pq.empty()) {
        res.push_back(pq.top());
        pq.pop();
    }

    sort(res.begin(), res.end(), [](const SlotCount& a, const SlotCount& b) {
        if (a.count != b.count) return a.count > b.count;
        if (a.zone != b.zone) return a.zone < b.zone;
        return a.hour < b.hour;
    });

    if ((int)res.size() > k) res.resize(k);
    return res;
}

int main() {
    TripAnalyzer analyzer;
    analyzer.ingestStdin();

    cout << "TOP_ZONES\n";
    for (auto& z : analyzer.topZones())
        cout << z.zone << "," << z.count << "\n";

    cout << "TOP_SLOTS\n";
    for (auto& s : analyzer.topBusySlots())
        cout << s.zone << "," << s.hour << "," << s.count << "\n";

    return 0;
}

