/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 * All rights reserved.
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "AbstractConfig.h"

#include <assert.h>
#include <chrono>
#include <functional>
#include <set>
#include <string>
#include <vector>

namespace KINETO_NAMESPACE {

class Config : public AbstractConfig {
 public:
  Config();
  Config& operator=(const Config&) = delete;
  Config(Config&&) = delete;
  Config& operator=(Config&&) = default;

  // Return a full copy including feature config object
  std::unique_ptr<Config> clone() {
    auto cfg = std::unique_ptr<Config>(new Config(*this));
    cloneFeaturesInto(*cfg);
    return cfg;
  }

  bool handleOption(const std::string& name, std::string& val) override;

  // Log events to this file
  const std::string& eventLogFile() const {
    return eventLogFile_;
  }

  bool activityProfilerEnabled() const {
    return activityProfilerEnabled_;
  }

  // Log activitiy trace to this file
  const std::string& activitiesLogFile() const {
    return activitiesLogFile_;
  }

  // Is profiling enabled for the given device?
  bool eventProfilerEnabledForDevice(uint32_t dev) const {
    return 0 != (eventProfilerDeviceMask_ & (1 << dev));
  }

  // Take a sample (read hardware counters) at this frequency.
  // This controls how often counters are read - if all counters cannot
  // be collected simultaneously then multiple samples are needed to
  // collect all requested counters - see multiplex period.
  std::chrono::milliseconds samplePeriod() const {
    return samplePeriod_;
  }

  void setSamplePeriod(std::chrono::milliseconds period) {
    samplePeriod_ = period;
  }

  // When all requested counters cannot be collected simultaneously,
  // counters will be multiplexed at this frequency.
  // Multiplexing can have a large performance impact if done frequently.
  // To avoid a perf impact, keep this at 1s or above.
  std::chrono::milliseconds multiplexPeriod() const {
    return multiplexPeriod_;
  }

  void setMultiplexPeriod(std::chrono::milliseconds period) {
    multiplexPeriod_ = period;
  }

  // Report counters at this frequency. Note that several samples can
  // be reported each time, see samplesPerReport.
  std::chrono::milliseconds reportPeriod() const {
    return reportPeriod_;
  }

  void setReportPeriod(std::chrono::milliseconds msecs);

  // Number of samples dispatched each report period.
  // Must be in the range [1, report period / sample period].
  // In other words, aggregation is supported but not interpolation.
  int samplesPerReport() const {
    return samplesPerReport_;
  }

  void setSamplesPerReport(int count) {
    samplesPerReport_ = count;
  }

  // The names of events to collect
  const std::set<std::string>& eventNames() const {
    return eventNames_;
  }

  // Add additional events to be profiled
  void addEvents(const std::set<std::string>& names) {
    eventNames_.insert(names.begin(), names.end());
  }

  // The names of metrics to collect
  const std::set<std::string>& metricNames() const {
    return metricNames_;
  }

  // Add additional metrics to be profiled
  void addMetrics(const std::set<std::string>& names) {
    metricNames_.insert(names.begin(), names.end());
  }

  const std::vector<int>& percentiles() const {
    return eventReportPercentiles_;
  }

  // Profile for this long, then revert to base config
  std::chrono::seconds eventProfilerOnDemandDuration() const {
    return eventProfilerOnDemandDuration_;
  }

  void setEventProfilerOnDemandDuration(std::chrono::seconds duration) {
    eventProfilerOnDemandDuration_ = duration;
  }

  // Too many event profilers on a single system can overload the driver.
  // At some point, latencies shoot through the roof and collection of samples
  // becomes impossible. To avoid this situation we have a limit of profilers
  // per GPU.
  // NOTE: Communication with a daemon is needed for this feature.
  // Library must be built with an active DaemonConfigLoader.
  int maxEventProfilersPerGpu() const {
    return eventProfilerMaxInstancesPerGpu_;
  }

  // Trace for this long
  std::chrono::milliseconds activitiesOnDemandDuration() const {
    return activitiesOnDemandDuration_;
  }

  std::chrono::milliseconds activitiesOnDemandDurationDefault() const;

  void setActivitiesOnDemandDuration(std::chrono::milliseconds duration) {
    activitiesOnDemandDuration_ = duration;
  }

  // Trace for this many iterations, determined by external API
  int activitiesOnDemandExternalIterations() const {
    return activitiesExternalAPIIterations_;
  }

  const std::string& activitiesOnDemandExternalTarget() const {
    return activitiesExternalAPIIterationsTarget_;
  }

  const std::vector<std::string>& activitiesOnDemandExternalFilter() const {
    return activitiesExternalAPIFilter_;
  }

  // Only profile nets with at least this many operators.
  // Controlled by external API.
  int activitiesOnDemandExternalNetSizeThreshold() const {
    return activitiesExternalAPINetSizeThreshold_;
  }

  // Only profile nets with at least this many GPU operators.
  // Controlled by external API.
  int activitiesOnDemandExternalGpuOpCountThreshold() const {
    return activitiesExternalAPIGpuOpCountThreshold_;
  }

  int activitiesMaxGpuBufferSize() const {
    return activitiesMaxGpuBufferSize_;
  }

  std::chrono::seconds activitiesWarmupDuration() const {
    return activitiesWarmupDuration_;
  }

  // Request was initiated at this time
  const std::chrono::time_point<std::chrono::system_clock> requestTimestamp()
      const {
    return requestTimestamp_;
  }

  bool hasRequestTimestamp() const {
    return requestTimestamp_.time_since_epoch().count() > 0;
  }

  const std::chrono::seconds maxRequestAge() const;

  // All VLOG* macros will log if the verbose log level is >=
  // the verbosity specified for the verbose log message.
  // Default value is -1, so messages with log level 0 will log by default.
  int verboseLogLevel() const {
    return verboseLogLevel_;
  }

  // Modules for which verbose logging is enabled.
  // If empty, logging is enabled for all modules.
  const std::vector<std::string>& verboseLogModules() const {
    return verboseLogModules_;
  }

  bool sigUsr2Enabled() const {
    return enableSigUsr2_;
  }

  static std::chrono::milliseconds alignUp(
      std::chrono::milliseconds duration,
      std::chrono::milliseconds alignment) {
    duration += alignment;
    return duration - (duration % alignment);
  }

  std::chrono::time_point<std::chrono::high_resolution_clock>
  eventProfilerOnDemandStartTime() const {
    return eventProfilerOnDemandTimestamp_;
  }

  std::chrono::time_point<std::chrono::high_resolution_clock>
  eventProfilerOnDemandEndTime() const {
    return eventProfilerOnDemandTimestamp_ + eventProfilerOnDemandDuration_;
  }

  std::chrono::time_point<std::chrono::high_resolution_clock>
  activityProfilerRequestReceivedTime() const {
    return activitiesOnDemandTimestamp_;
  }

  void updateActivityProfilerRequestReceivedTime();

  void printActivityProfilerConfig(std::ostream& s) const override;

  void validate();

  static void addConfigFactory(
      std::string name,
      std::function<AbstractConfig*(const Config&)> factory);

  void print(std::ostream& s) const;

 private:
  explicit Config(const Config& other) = default;

  AbstractConfig* cloneDerived() const override {
    // Clone from AbstractConfig not supported
    assert(false);
    return nullptr;
  }

  uint8_t createDeviceMask(const std::string& val);

  int verboseLogLevel_;
  std::vector<std::string> verboseLogModules_;

  // Event profiler
  // These settings are also supported in on-demand mode
  std::chrono::milliseconds samplePeriod_;
  std::chrono::milliseconds reportPeriod_;
  int samplesPerReport_;
  std::set<std::string> eventNames_;
  std::set<std::string> metricNames_;

  // On-demand duration
  std::chrono::seconds eventProfilerOnDemandDuration_;
  // Last on-demand request
  std::chrono::time_point<std::chrono::high_resolution_clock>
      eventProfilerOnDemandTimestamp_;

  int eventProfilerMaxInstancesPerGpu_;

  // These settings can not be changed on-demand
  std::string eventLogFile_;
  std::vector<int> eventReportPercentiles_ = {5, 25, 50, 75, 95};
  uint8_t eventProfilerDeviceMask_ = ~0;
  std::chrono::milliseconds multiplexPeriod_;

  // Activity profiler
  bool activityProfilerEnabled_;

  // The activity profiler settings are all on-demand
  std::string activitiesLogFile_;

  int activitiesMaxGpuBufferSize_;
  std::chrono::seconds activitiesWarmupDuration_;

  // Profile for specified iterations and duration
  std::chrono::milliseconds activitiesOnDemandDuration_;
  int activitiesExternalAPIIterations_;
  // Use this net name for iteration count
  std::string activitiesExternalAPIIterationsTarget_;
  // Only profile nets that includes this in the name
  std::vector<std::string> activitiesExternalAPIFilter_;
  // Only profile nets with at least this many operators
  int activitiesExternalAPINetSizeThreshold_;
  // Only profile nets with at least this many GPU operators
  int activitiesExternalAPIGpuOpCountThreshold_;
  // Last activity profiler request
  std::chrono::time_point<std::chrono::high_resolution_clock>
      activitiesOnDemandTimestamp_;

  // Synchronized start timestamp
  std::chrono::time_point<std::chrono::system_clock> requestTimestamp_;

  // Enable profiling via SIGUSR2
  bool enableSigUsr2_;
};

} // namespace KINETO_NAMESPACE
