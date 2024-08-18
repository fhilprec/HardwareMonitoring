#include "FileManager.h"

#include <utility>

#include "Counter.hpp"
#include "fmt/format.h"

FileManager::FileManager(const std::vector<std::shared_ptr<IDevice>> &devices,
                         const std::filesystem::path &tempOutputDirectory,
                         const std::filesystem::path &outputDirectory, bool saveRawResults)
        : tempOutputDirectory(tempOutputDirectory), outputDirectory(outputDirectory), showRawMetricsInResult(saveRawResults) {
    if (!is_directory(outputDirectory)) {
        throw std::invalid_argument(fmt::format("Path '{}' is not a directory",
                                                outputDirectory.string()));
    }

    if (!is_directory(tempOutputDirectory)) {
        throw std::invalid_argument(fmt::format("Path '{}' is not a directory",
                                                tempOutputDirectory.string()));
    }

    for (const auto &device: devices) {
        auto path = tempOutputDirectory;
        filePathsForDevices.emplace(device, path.append(fmt::format("{}_raw.csv", device->getName())));
    }
    for (const auto &[device, deviceFilePath]: filePathsForDevices) {
        auto ofstream = std::make_shared<std::ofstream>(std::ofstream(deviceFilePath, std::ios_base::trunc));
        tempDataForDevices.emplace(device, ofstream);

        std::vector<Metric> userMetrics = device->getUserMetrics();
        std::vector<Metric> counterMetrics = Counter::getAdditionalMetricsAdded();

        std::vector<Metric> headerMetrics;
        headerMetrics.reserve(userMetrics.size() + counterMetrics.size());
        for (const auto &userMetric: userMetrics) {
            if (userMetric.samplingMethod != CALCULATED) {
                headerMetrics.push_back(userMetric);
            }
        }
        headerMetrics.insert(headerMetrics.end(), counterMetrics.begin(), counterMetrics.end());

        const std::string header = createHeaderString(headerMetrics);
        ofstream->write(header.c_str(), header.size());
    }
}

void FileManager::writeToBuffer(const std::shared_ptr<IDevice> &device,
                                const std::vector<std::pair<Metric, Measurement>> &line) {
    const std::string stringLine = lineToString(line);
    tempDataForDevices.at(device)->write(stringLine.c_str(), stringLine.size());
}

std::vector<std::unordered_map<Metric, Measurement>>
FileManager::readAllFromBuffer(const std::shared_ptr<IDevice> &device) const {
    std::vector<std::unordered_map<Metric, Measurement>> measurements;
    tempDataForDevices.at(device)->close();
    std::ifstream fileBuffer(filePathsForDevices.at(device));

    std::string currentLine;
    std::getline(fileBuffer, currentLine);

    std::vector<Metric> metrics = readMetrics(device, currentLine);
    while (std::getline(fileBuffer, currentLine)) {
        measurements.push_back(stringToLine(currentLine, metrics));
    }
    return measurements;
}

void FileManager::save(
        std::unordered_map<std::shared_ptr<IDevice>, std::unordered_map<SamplingMethod, std::vector<std::unordered_map<Metric, Measurement>>>>
        data) {
    for (const auto &[device, ignored]: filePathsForDevices) {
        auto path = outputDirectory;
        std::filesystem::path filePath = path.append(fmt::format("{}.csv", device->getName()));
        auto output = std::make_shared<std::ofstream>(std::ofstream(std::ofstream(filePath)));

        std::vector<Metric> userMetrics = device->getUserMetrics();
        if (!showRawMetricsInResult) {
            std::erase_if(userMetrics, [](const Metric &metric) { return metric.raw; });
        }
        std::vector<Metric> counterMetrics = Counter::getAdditionalMetricsAdded();

        std::vector<Metric> headerMetrics;
        headerMetrics.reserve(userMetrics.size() + counterMetrics.size());
        headerMetrics.insert(headerMetrics.end(), userMetrics.begin(), userMetrics.end());
        headerMetrics.insert(headerMetrics.end(), counterMetrics.begin(), counterMetrics.end());


        const std::string header = createHeaderString(headerMetrics);
        output->write(header.c_str(), header.size());
        for (const auto &[sampler, rows]: data.at(device)) {
            for (const auto &row: rows) {
                if (row.empty()) continue;
                if (!showRawMetricsInResult) {
                    bool allRaw = true;
                    for (const auto &[metric, measurement]: row) {
                        allRaw &= metric.raw;
                    }
                    if (allRaw) continue;
                }
                std::string line = lineToStringOrdered(row, headerMetrics);
                if (!line.empty()) output->write(line.c_str(), line.size());
            }
        }
    }
}

std::string FileManager::createHeaderString(const std::vector<Metric> &metrics) {
    std::string res;
    res.reserve(metrics.size() * (32 + 1));
    for (const auto &metric: metrics) {
        res.append(metric.name);
        res.append(";");
    }
    res.pop_back();
    res.append("\n");
    return res;
}

std::string FileManager::lineToString(const std::vector<std::pair<Metric, Measurement>> &line) {
    std::string res;
    res.reserve(line.size() * (32 + 1));
    for (const auto &metricMeasurements: line) {
        res.append(metricMeasurements.second.value);
        res.append(";");
    }
    res.pop_back();
    res.append("\n");
    return res;
}

std::string FileManager::lineToStringOrdered(const std::unordered_map<Metric, Measurement> &line,
                                             const std::vector<Metric> &metricsOrdered) {
    std::string res;
    res.reserve(line.size() * (32 + 1));
    for (const auto &metric: metricsOrdered) {
        if (line.contains(metric))res.append(line.at(metric).value);
        res.append(";");
    }
    res.pop_back();
    res.append("\n");
    return res.size() == metricsOrdered.size() ? "" : res;
}

std::unordered_map<Metric, Measurement> FileManager::stringToLine(const std::string &line,
                                                                  const std::vector<Metric> &readMetrics) {
    std::unordered_map<Metric, Measurement> res;
    res.reserve(readMetrics.size());
    std::stringstream ss(line);
    std::string currentLine;

    size_t index = 0;
    while (std::getline(ss, currentLine, ';')) {
        res[readMetrics[index++]] = Measurement(currentLine);
    }
    return res;
}

std::vector<Metric> FileManager::readMetrics(const std::shared_ptr<IDevice> &device, const std::string &line) {
    std::vector<Metric> res;
    std::stringstream ss(line);
    std::string currentLine;

    while (std::getline(ss, currentLine, ';')) {
        for (auto &allowedMetric: device->getUserMetrics()) {
            if (allowedMetric.name == currentLine) {
                res.push_back(allowedMetric);
                continue;
            }
        }
        for (auto &counterMetric: Counter::getAdditionalMetricsAdded()) {
            if (counterMetric.name == currentLine) {
                res.push_back(counterMetric);
                continue;
            }
        }
    }
    return res;
}
