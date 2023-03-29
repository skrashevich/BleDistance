#ifndef BEACON_TRACKER_H
#define BEACON_TRACKER_H

#include <string>
#include <vector>
#include <time.h>
#include <math.h>
#include <algorithm>

#include "esp32_ble_tracker.h"
#include "esp_log.h"
#include "one_euro_filter.h"

#define TIMEOUT 120
#define BUF_SIZE 9
#define MAX_DIST NAN
#define MIN_RSSI NAN

#define FCMIN 0.0001
#define BETA 0.05
#define DCUTOFF 1.0

#define PATH_LOSS 2.7

class BeaconTracker {

public:
    std::string name;
    esphome::esp32_ble_tracker::ESPBTUUID uuid;

private:
    OneEuro filter;
    std::vector<float> dist_buf;
    std::vector<time_t> time_buf;
    time_t now;
    int rssi;
    float filt_dist;

public:
    BeaconTracker(std::string n, std::string u, float fcmin, float beta);

private:
    static std::string reverse_bytes(std::string uuid);
    void validate();

public:
    void update(int r, int p);
    float get_rssi();
    float get_raw_dist();
    float get_dist();

};

void parseAdvertisement(esphome::esp32_ble_tracker::ESPBTDevice dev);
void addTracker(std::string n, std::string u, float fcmin = FCMIN, float beta = BETA);
BeaconTracker& getTracker(std::string n);

#endif
