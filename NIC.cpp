#include "NIC.h"


static const std::vector<Metric> METRICS{
    Metric(POLLING, "port_rcv_data", true)
    Metric(POLLING, "port_xmit_data", true)
    Metric(POLLING, "port_rcv_packets", true)
    Metric(POLLING, "port_xmit_packets", true)
}

NIC::NIC(const std::string& nicName, int port, double interval)
    : nicName(nicName), port(port), interval(interval) {
    hwCountersPath = fmt::format("/sys/class/infiniband/{}/ports/{}/hw_counters/", nicName, port);
    portCountersPath = fmt::format("/sys/class/infiniband/{}/ports/{}/counters/", nicName, port);
    loadCounters();
}

void NIC::loadCounters() {
    for (const auto& entry : std::filesystem::directory_iterator(hwCountersPath)) {
        if (std::filesystem::is_regular_file(entry.path())) {
            counters[entry.path().filename().string()] = entry.path();
            auto it = std::find_if(METRICS.begin(), METRICS.end(), [&entry](const Metric& m) {return m.name == entry.path().filename().string()});
            if(it != METRICS.end()){
                std::ifstream file(entry.path());
                if (file.is_open()) {
                    uint64_t value;
                    file >> value;
                    if(endsWith(entry.path().filename().string(), "_data")) value = value*4;
                    double d = static_cast<double>(value);
                    d = d * (1/interval);
                    value = static_cast<uint64_t>(d);
                    lastValues.emplace(*it.name, value);
                }
            }
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(portCountersPath)) {
        if (std::filesystem::is_regular_file(entry.path())) {
            counters[entry.path().filename()] = entry.path();
            auto it = find_if(METRICS.begin(), METRICS.end(), [&entry.path().filename()](const Metric& m) {return m.name == entry.path().filename();})
            if(it != METRICS.end()){
                std::ifstream file(entry.path());
                if (file.is_open()) {
                    uint64_t value;
                    file >> value;
                    if(endsWith(entry.path().filename().string(), "_data")) value = value*4;
                    double d = static_cast<double>(value);
                    d = d * (1/interval);
                    value = static_cast<uint64_t>(d);
                    lastValues.emplace(*it.name, value);
                }
            }
        }
    }
}

static bool endsWith(const std::string& str, const std::string& suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}

std::unordered_map<std::string, uint64_t> NIC::readCounters() {
    std::unordered_map<std::string, uint64_t> counterValues;
    for (const auto& [name, path] : counters) {
        std::ifstream file(path);
        if (file.is_open()) {
            uint64_t value;
            file >> value;
            if(endsWith(name, "_data")) value = value*4;
            double d = static_cast<double>(value);
            d = d * (1/interval);
            value = static_cast<uint64_t>(d);
            counterValues[name] = value;
        }
    }
    return counterValues;
}

std::vector<std::pair<Metric, Measurement>> NIC::getData(SamplingMethod sampler) {
    auto result = Device::getData(sampler);
    return result;
}

Measurement NIC::fetchMetric(const Metric& metric) {
    auto counterValues = readCounters();
    Measurement tmp = Measurement(std::to_string(counterValues[metric.name]-lastValues[metric.name]))
    lastValues[metric.name] = counterValues[metric.name];
    return tmp;
}

Measurement NIC::calculateMetric(const Metric& metric, const std::unordered_map<std::string, std::unordered_map<SamplingMethod, std::vector<std::vector<std::pair<Metric, Measurement>>>>>& requestedMetricsByDeviceBySamplingMethod) {
    return Measurement("0");
}

std::string NIC::getDeviceName() {
    return fmt::format("NIC_{}_{}", nicName, port);
}

std::unordered_map<std::string, Metric> NIC::getAllDeviceMetricsByName() {
    std::unordered_map<std::string, Metric> result;
    for (const auto& metric : METRICS) {
        result.emplace(metric.name, metric);
    }
    return result;
}

std::unordered_map<std::string, std::vector<Metric>> NIC::getNeededMetricsForCalculatedMetrics(const Metric& metric) {
    return {};
}
