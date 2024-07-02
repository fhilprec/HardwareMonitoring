#include "FileManager.h"

#include "Counter.hpp"

FileManager::FileManager(const std::vector<std::shared_ptr<Device>>& devices,
                         const std::optional<std::filesystem::path>& outputDirectory) : outputDirectory(outputDirectory)
{
    if (outputDirectory.has_value() && !is_directory(outputDirectory.value()))
    {
        throw std::invalid_argument(std::format("Path '{}' is not a directory",
                                                outputDirectory.value().string()));
    }

    std::filesystem::path tempPath = std::filesystem::temp_directory_path().append("HardwareMonitoring");
    if (!is_directory(tempPath)) create_directory(tempPath);
    for (const auto& device : devices)
    {
        filePathsForDevices.emplace(*device, tempPath.append(std::format("{}_raw.csv", device->getName())));
    }
    for (const auto& [device, deviceFilePath] : filePathsForDevices)
    {
        auto ofstream = std::make_shared<std::ofstream>(std::ofstream(deviceFilePath, std::ios_base::trunc));
        tempDataForDevices.emplace(device, ofstream);

        std::vector<Metric> userMetrics = device.getUserMetrics();
        std::vector<Metric> counterMetrics = Counter::getAdditionalMetricsAdded();

        std::vector<Metric> headerMetrics;
        headerMetrics.reserve(userMetrics.size() + counterMetrics.size());
        for (const auto& userMetric : userMetrics)
        {
            if (userMetric.samplingMethod != CALCULATED)
            {
                headerMetrics.push_back(userMetric);
            }
        }
        headerMetrics.insert(headerMetrics.end(), counterMetrics.begin(), counterMetrics.end());

        const std::string header = createHeaderString(headerMetrics);
        ofstream->write(header.c_str(), header.size());
    }
}

void FileManager::writeToBuffer(const Device& device, const std::vector<std::pair<Metric, Measurement>>& line)
{
    const std::string stringLine = lineToString(line);
    tempDataForDevices.at(device)->write(stringLine.c_str(), stringLine.size());
}

std::vector<std::vector<std::pair<Metric, Measurement>>> FileManager::readAllFromBuffer(const Device& device) const
{
    std::vector<std::vector<std::pair<Metric, Measurement>>> measurements;
    tempDataForDevices.at(device)->close();
    std::ifstream fileBuffer(filePathsForDevices.at(device));

    std::string currentLine;
    std::getline(fileBuffer, currentLine);

    std::vector<Metric> metrics = readMetrics(device, currentLine);
    while (std::getline(fileBuffer, currentLine))
    {
        measurements.push_back(stringToLine(currentLine, metrics));
    }
    return measurements;
}

void FileManager::save(
    std::unordered_map<Device, std::unordered_map<Sampler, std::vector<std::vector<std::pair<Metric, Measurement>>>>>
    data)
{
    for (const auto& [device, ignored] : filePathsForDevices)
    {
        std::shared_ptr<std::ostream> output(&std::cout, [](void*)
        {
        });
        if (outputDirectory.has_value())
        {
            std::filesystem::path filePath = outputDirectory.value().append(std::format("{}.csv", device.getName()));
            output = std::make_shared<std::ofstream>(std::ofstream(std::ofstream(filePath)));
        }

        std::vector<Metric> userMetrics = device.getUserMetrics();
        std::vector<Metric> counterMetrics = Counter::getAdditionalMetricsAdded();

        std::vector<Metric> headerMetrics;
        headerMetrics.reserve(userMetrics.size() + counterMetrics.size());
        headerMetrics.insert(headerMetrics.end(), userMetrics.begin(), userMetrics.end());
        headerMetrics.insert(headerMetrics.end(), counterMetrics.begin(), counterMetrics.end());


        const std::string header = createHeaderString(headerMetrics);
        output->write(header.c_str(), header.size());
        for (const auto& [sampler, rows] : data.at(device))
        {
            for (const auto& row : rows)
            {
                if (row.empty()) continue;
                std::string line = lineToStringOrdered(row, headerMetrics);
                if (!line.empty()) output->write(line.c_str(), line.size());
            }
        }
    }
}

std::string FileManager::createHeaderString(const std::vector<Metric>& metrics)
{
    std::string res;
    res.reserve(metrics.size() * (32 + 1));
    for (const auto& metric : metrics)
    {
        res.append(metric.name);
        res.append(";");
    }
    res.pop_back();
    res.append("\n");
    return res;
}

std::string FileManager::lineToString(const std::vector<std::pair<Metric, Measurement>>& line)
{
    std::string res;
    res.reserve(line.size() * (32 + 1));
    for (const auto& metricMeasurements : line)
    {
        res.append(metricMeasurements.second.value);
        res.append(";");
    }
    res.pop_back();
    res.append("\n");
    return res;
}

std::string FileManager::lineToStringOrdered(const std::vector<std::pair<Metric, Measurement>>& line,
                                             const std::vector<Metric>& metricsOrdered)
{
    std::string res;
    res.reserve(line.size() * (32 + 1));
    for (const auto& metric : metricsOrdered)
    {
        for (const auto& [lineMetric, measurement] : line)
        {
            if (metric == lineMetric)
            {
                res.append(measurement.value);
                continue;
            }
        }

        res.append(";");
    }
    res.pop_back();
    res.append("\n");
    return res.size() == metricsOrdered.size() ? "" : res;
}

std::vector<std::pair<Metric, Measurement>> FileManager::stringToLine(const std::string& line,
                                                                      const std::vector<Metric>& readMetrics)
{
    std::vector<std::pair<Metric, Measurement>> res;
    res.reserve(readMetrics.size());
    std::stringstream ss(line);
    std::string currentLine;

    size_t index = 0;
    while (std::getline(ss, currentLine, ';'))
    {
        res.emplace_back(readMetrics[index++], Measurement(currentLine));
    }
    return res;
}

std::vector<Metric> FileManager::readMetrics(const Device& device, const std::string& line)
{
    std::vector<Metric> res;
    std::stringstream ss(line);
    std::string currentLine;

    while (std::getline(ss, currentLine, ';'))
    {
        for (auto& allowedMetric : device.getUserMetrics())
        {
            if (allowedMetric.name == currentLine)
            {
                res.push_back(allowedMetric);
                continue;
            }
        }
        for (auto& counterMetric : Counter::getAdditionalMetricsAdded())
        {
            if (counterMetric.name == currentLine)
            {
                res.push_back(counterMetric);
                continue;
            }
        }
    }
    return res;
}
