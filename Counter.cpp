#include "Counter.hpp"

Counter::Counter(const CounterConfig counterConfig, FileManager& fileManager):
    counterConfig(counterConfig),
    fileManager(fileManager),
    pollingThread(std::jthread(std::bind_front(&Counter::poll, this))),
    slowPollingDevices(std::vector<Device>(counterConfig.devices.size()))
{
}

void Counter::start()
{
    started = true;
    startCondition.notify_all();
}

void Counter::stop()
{
    pollingThread.request_stop();
    pollingThread.join();
    started = false;
    startCondition.notify_all();
}

std::vector<Metric> Counter::getAdditionalMetricsAdded()
{
    return {TIME_TAKEN_POLLING_METRIC, TIME_METRIC, SAMPLING_METHOD_METRIC};
}

void Counter::poll(const std::stop_token& stop_token)
{
    std::unique_lock lk(startLock);
    startCondition.wait(lk, [this] { return started; });

    fetchData(TWO_SHOT);

    while (!stop_token.stop_requested())
    {
        auto startTimePoll = std::chrono::system_clock::now();
        fetchData(POLLING);
        auto endTimePoll = std::chrono::system_clock::now();

        auto remainingPollTime = counterConfig.pollingTimeFrame - std::chrono::duration_cast<
            std::chrono::milliseconds>(endTimePoll - startTimePoll);
        remainingPollTime = remainingPollTime < std::chrono::milliseconds(0)
                                ? counterConfig.pollingTimeFrame - remainingPollTime
                                : remainingPollTime;

        std::this_thread::sleep_for(remainingPollTime);
    }

    fetchData(TWO_SHOT);
    fetchData(ONE_SHOT);
}

void Counter::fetchData(Sampler sampleMethod)
{
    for (const auto& device : counterConfig.devices)
    {
        auto startPoll = std::chrono::system_clock::now();
        auto deviceData = device->getData(sampleMethod);
        auto endPoll = std::chrono::system_clock::now();

        auto timeForPull = std::chrono::duration_cast<std::chrono::milliseconds>(endPoll - startPoll);

#if sampleMethod == POLLING
        checkPollingTime(*device, timeForPull);
#endif

        if (!deviceData.empty())
        {
            deviceData.emplace_back(TIME_TAKEN_POLLING_METRIC, std::format("{} ms", timeForPull.count()));
            deviceData.emplace_back(TIME_METRIC, std::format("{:%Y/%m/%d %T}", startPoll));
            deviceData.emplace_back(SAMPLING_METHOD_METRIC, getDisplayForSampler(sampleMethod));
            fileManager.writeToBuffer(*device, deviceData);
        }
    }
}

void Counter::checkPollingTime(const Device& device, const std::chrono::milliseconds& timeForPull)
{
    const bool deviceNotWarned = std::ranges::find(slowPollingDevices, device) != slowPollingDevices.end();

    if (deviceNotWarned && timeForPull * counterConfig.devices.size() > counterConfig.pollingTimeFrame)
    {
        std::cerr << "Polling Time Window of " << counterConfig.pollingTimeFrame.count() << "ms may get skipped";
        std::cerr << "Device '" << device.getName() << "' took " << timeForPull.count() <<
            "ms for Poll, expecting Poll to get skipped";
        slowPollingDevices.push_back(device);
    }
}
