#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <array>


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
    void ingestStdin();
    vector<ZoneCount> topZones(int k = 10) const;
    vector<SlotCount> topBusySlots(int k = 10) const;

private:
    unordered_map<string, long long> zoneCounts;
    unordered_map<string, array<long long, 24>> slotCounts;

    static string trimCopy(const string& s) {
        size_t i = 0, j = s.size();
        while (i < j && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) i++;
        while (j > i && (s[j - 1] == ' ' || s[j - 1] == '\t' || s[j - 1] == '\r' || s[j - 1] == '\n')) j--;
        return s.substr(i, j - i);
    }

    static bool splitComma6(const string& line, string out[6]) {
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
        if (idx != 6) return false;
        for (int i = 0; i < 6; i++) out[i] = trimCopy(out[i]);
        return true;
    }

    static bool parseHour(const string& dt, int& hour) {
        string s = trimCopy(dt);
        size_t colon = s.find(':');
        if (colon == string::npos) return false;
        if (colon == 0) return false;

        size_t p = colon;
        while (p > 0 && (s[p - 1] == ' ' || s[p - 1] == '\t')) p--;
        size_t end = p;
        while (p > 0 && isdigit(static_cast<unsigned char>(s[p - 1]))) p--;
        if (p == end) return false;

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
};


