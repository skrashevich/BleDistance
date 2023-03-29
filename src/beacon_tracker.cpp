#include "beacon_tracker.h"

#define TAG "ble_dist"

std::vector<BeaconTracker> trackers;

BeaconTracker::BeaconTracker(std::string n, std::string u, float fcmin, float beta)
    : name(n)
    , uuid(esphome::esp32_ble_tracker::ESPBTUUID::from_raw(reverse_bytes(u)))
    , filter(1.0, fcmin, beta, DCUTOFF)
{
    dist_buf.reserve(BUF_SIZE + 1);
    time_buf.reserve(BUF_SIZE + 1);
}

std::string BeaconTracker::reverse_bytes(std::string uuid) {
    uuid.erase(std::remove(uuid.begin(), uuid.end(), '-'), uuid.end());
    std::string out = uuid;
    size_t len = uuid.length();
    for (size_t i = 0; i < len; i += 2) {
        out[i] = uuid[len - i - 2];
        out[i + 1] = uuid[len - i - 1];
    }
    out.insert(8, "-");
    out.insert(13, "-");
    out.insert(18, "-");
    out.insert(23, "-");
    return out;
}

void BeaconTracker::validate() {
    now = esp_timer_get_time();
    while (!time_buf.empty() && (time_buf.size() > BUF_SIZE || now - time_buf.back() > TIMEOUT * 1e6)) {
        dist_buf.pop_back();
        time_buf.pop_back();
    }
}

void BeaconTracker::update(int r, int p) {
    now = esp_timer_get_time();
    rssi = r;
    dist_buf.insert(dist_buf.begin(), pow(10.0, (p - r) / (10.0 * PATH_LOSS)));
    time_buf.insert(time_buf.begin(), now);
    validate();
    auto temp = dist_buf;
    std::sort(temp.begin(), temp.end());
    filt_dist = filter(temp[std::max(0u, (unsigned)(dist_buf.size()/ 3.0 - 1))], now);
std::string s = "";
for (auto b : temp)
s += esphome::to_string(b) + ", ";
ESP_LOGD(TAG, "Recognized %s iBeacon: %s", name.c_str(), uuid.to_string().c_str());
ESP_LOGD(TAG, " RSSI: %d", r);
}

float BeaconTracker::get_rssi() {
validate();
return time_buf.empty() ? MIN_RSSI : std::max((float)rssi, MIN_RSSI);
}

float BeaconTracker::get_raw_dist() {
validate();
return time_buf.empty() ? MAX_DIST : std::min(dist_buf[0], MAX_DIST);
}

float BeaconTracker::get_dist() {
validate();
return time_buf.empty() ? MAX_DIST : std::min(filt_dist, MAX_DIST);
}

void parseAdvertisement(esphome::esp32_ble_tracker::ESPBTDevice dev) {
if (dev.get_ibeacon().has_value()) {
auto ib = dev.get_ibeacon().value();
auto uuid = ib.get_uuid();
auto txpwr = ib.get_signal_power();
auto rssi = dev.get_rssi();
for (auto& t : trackers)
if (t.uuid == uuid)
t.update(rssi, txpwr);
}
}

void addTracker(std::string n, std::string u, float fcmin, float beta) {
trackers.emplace(trackers.end(), n, u, fcmin, beta);
}

BeaconTracker& getTracker(std::string n) {
for (auto& t : trackers)
if (t.name == n)
return t;
ESP_LOGW(TAG, "BeaconTracker %s not recognized, returning the first tracker (hopefully one exists)", n.c_str());
return trackers[0];
}