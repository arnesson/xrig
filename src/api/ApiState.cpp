/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2016-2017 XMRig       <support@xmrig.com>
 *
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <string.h>
#include <uv.h>
#include <map>
#include <vector>

#if _WIN32
#   include "winsock2.h"
#else
#   include "unistd.h"
#endif


#include "amd/Adl.h"
#include "api/ApiState.h"
#include "Cpu.h"
#include "net/Job.h"
#include "net/Url.h"
#include "Options.h"
#include "Platform.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "workers/Workers.h"
#include "workers/Hashrate.h"
#include "workers/Handle.h"

extern "C"
{
#include "crypto/c_keccak.h"
}


static inline double normalize2(double d)
{
    if (!isnormal(d)) {
        return 0.0;
    }

    return floor(d * 100.0) / 100.0;
}


static inline double normalize3(double d)
{
    if (!isnormal(d)) {
        return 0.0;
    }

    return floor(d * 1000.0) / 1000.0;
}


ApiState::ApiState()
{
    m_threads  = (int) Options::i()->threads().size();
    m_hashrate = new double[m_threads * 3]();

    memset(m_totalHashrate, 0, sizeof(m_totalHashrate));
    memset(m_id, 0, sizeof(m_id));

    if (Options::i()->id()) {
        strncpy(m_id, Options::i()->id(), sizeof(m_id) - 1);
    }
    else {
        gethostname(m_id, sizeof(m_id) - 1);
    }
}


ApiState::~ApiState()
{
    delete [] m_hashrate;
}


char *ApiState::get(const char *url, int *status) const
{
    rapidjson::Document doc;
    doc.SetObject();

    getId(doc);
    getMiner(doc);
    getHashrate(doc);
    getGpus(doc);
    getResults(doc);
    getConnection(doc);

    return finalize(doc);
}


void ApiState::tick(const Hashrate *hashrate)
{
    for (int i = 0; i < m_threads; ++i) {
        m_hashrate[i * 3]     = hashrate->calc((size_t) i, Hashrate::ShortInterval);
        m_hashrate[i * 3 + 1] = hashrate->calc((size_t) i, Hashrate::MediumInterval);
        m_hashrate[i * 3 + 2] = hashrate->calc((size_t) i, Hashrate::LargeInterval);
    }

    m_totalHashrate[0] = hashrate->calc(Hashrate::ShortInterval);
    m_totalHashrate[1] = hashrate->calc(Hashrate::MediumInterval);
    m_totalHashrate[2] = hashrate->calc(Hashrate::LargeInterval);
    m_highestHashrate  = hashrate->highest();
}


void ApiState::tick(const NetworkState &network)
{
    m_network = network;
}


char *ApiState::finalize(rapidjson::Document &doc) const
{
    rapidjson::StringBuffer buffer(0, 4096);
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    writer.SetMaxDecimalPlaces(10);
    doc.Accept(writer);

    return strdup(buffer.GetString());
}


void ApiState::getConnection(rapidjson::Document &doc) const
{
    auto &allocator = doc.GetAllocator();

    rapidjson::Value connection(rapidjson::kObjectType);
    connection.AddMember("pool",      rapidjson::StringRef(m_network.pool), allocator);
    connection.AddMember("user",      rapidjson::StringRef(m_network.userMask), allocator);
    connection.AddMember("uptime",    m_network.connectionTime(), allocator);
    connection.AddMember("ping",      m_network.latency(), allocator);
    connection.AddMember("failures",  m_network.failures, allocator);
    connection.AddMember("error_log", rapidjson::Value(rapidjson::kArrayType), allocator);

    doc.AddMember("connection", connection, allocator);
}


void ApiState::getGpus(rapidjson::Document &doc) const
{
    auto &allocator = doc.GetAllocator();

    rapidjson::Value gpus(rapidjson::kArrayType);

    for (const auto& map : Workers::gpus()) {
        std::vector<Handle*> handles = map.second;

        rapidjson::Value gpu(rapidjson::kObjectType);
        gpu.AddMember("bus_id", handles[0]->ctx()->busId, allocator);
        gpu.AddMember("adapter_index", Adl::i()->getAdapterIndex(handles[0]->ctx()->busId), allocator);
        gpu.AddMember("device_name", rapidjson::StringRef(handles[0]->ctx()->deviceName.c_str()), allocator);
        gpu.AddMember("temperature", Adl::i()->getTemperature(handles[0]->ctx()->busId) / 1000, allocator);

        const ADLODNPerformanceStatus* odNPerformanceStatus = Adl::i()->getPerformanceStatus(handles[0]->ctx()->busId);
        gpu.AddMember("core_clock", odNPerformanceStatus->iCoreClock / 100, allocator);
        gpu.AddMember("memory_clock", odNPerformanceStatus->iMemoryClock / 100, allocator);

        const ADLODNFanControl* odNFanControl = Adl::i()->getFanControl(handles[0]->ctx()->busId);
        gpu.AddMember("fan_speed", odNFanControl->iCurrentFanSpeed, allocator);

        const ADLODNPowerLimitSetting* odNPowerControl = Adl::i()->getPowerLimit(handles[0]->ctx()->busId);

        const ADLODNPerformanceLevelsX2* odNSystemClocks = Adl::i()->getSystemClocks(handles[0]->ctx()->busId);
        const ADLODNPerformanceLevelsX2* odNMemoryClocks = Adl::i()->getMemoryClocks(handles[0]->ctx()->busId);
        rapidjson::Value profile(rapidjson::kObjectType);
        rapidjson::Value system(rapidjson::kArrayType);
        rapidjson::Value memory(rapidjson::kArrayType);

        for (int i=0 ;i <ADL_PERFORMANCE_LEVELS; i++) {
            if (odNSystemClocks->aLevels[i].iEnabled) {
                rapidjson::Value level(rapidjson::kObjectType);
                level.AddMember("clock", odNSystemClocks->aLevels[i].iClock / 100, allocator);
                level.AddMember("vddc", normalize3(odNSystemClocks->aLevels[i].iVddc / 1000.0), allocator);
                system.PushBack(level, allocator);
            }
        }

        for (int i=0 ;i <ADL_PERFORMANCE_LEVELS; i++) {
            if (odNMemoryClocks->aLevels[i].iEnabled) {
                rapidjson::Value level(rapidjson::kObjectType);
                level.AddMember("clock", odNMemoryClocks->aLevels[i].iClock / 100, allocator);
                level.AddMember("vddc", normalize3(odNMemoryClocks->aLevels[i].iVddc / 1000.0), allocator);
                memory.PushBack(level, allocator);
            }
        }

        profile.AddMember("system", system, allocator);
        profile.AddMember("memory", memory, allocator);
        profile.AddMember("target_temperature", odNFanControl->iTargetTemperature, allocator);
        profile.AddMember("min_fan_limit", odNFanControl->iMinFanLimit, allocator);
        profile.AddMember("power_limit", odNPowerControl->iTDPLimit, allocator);

        gpu.AddMember("profile", profile, allocator);

        rapidjson::Value threads(rapidjson::kArrayType);

        double sum_hashrate_10s = 0.0;
        double sum_hashrate_60s = 0.0;
        double sum_hashrate_15m = 0.0;    

        for (const Handle *handle : handles) {
            rapidjson::Value thread(rapidjson::kObjectType);
            thread.AddMember("intensity", (uint64_t)handle->ctx()->rawIntensity, allocator);

            int i = handle->threadId() * 3;
            double hashrate_10s = normalize2(m_hashrate[i]);
            double hashrate_60s = normalize2(m_hashrate[i + 1]);
            double hashrate_15m = normalize2(m_hashrate[i + 2]);  

            thread.AddMember("hashrate_10s", hashrate_10s, allocator);
            thread.AddMember("hashrate_60s", hashrate_60s, allocator);
            thread.AddMember("hashrate_15m", hashrate_15m, allocator);

            sum_hashrate_10s += hashrate_10s;
            sum_hashrate_60s += hashrate_60s;
            sum_hashrate_15m += hashrate_15m;

            threads.PushBack(thread, allocator);
        }

        gpu.AddMember("threads", threads, allocator);

        gpu.AddMember("hashrate_10s", sum_hashrate_10s, allocator);
        gpu.AddMember("hashrate_60s", sum_hashrate_60s, allocator);
        gpu.AddMember("hashrate_15m", sum_hashrate_15m, allocator);

        gpus.PushBack(gpu, allocator);
    }

    doc.AddMember("gpus", gpus, allocator);
}


void ApiState::getHashrate(rapidjson::Document &doc) const
{
    auto &allocator = doc.GetAllocator();

    doc.AddMember("hashrate_10s", normalize2(m_totalHashrate[0]), allocator);
    doc.AddMember("hashrate_60s", normalize2(m_totalHashrate[1]), allocator);
    doc.AddMember("hashrate_15m", normalize2(m_totalHashrate[2]), allocator);
    doc.AddMember("hashrate_max", normalize2(m_highestHashrate), allocator);
}


void ApiState::getId(rapidjson::Document &doc) const
{
    doc.AddMember("id", rapidjson::StringRef(m_id), doc.GetAllocator());
}


void ApiState::getMiner(rapidjson::Document &doc) const
{
    auto &allocator = doc.GetAllocator();

    doc.AddMember("version",      rapidjson::StringRef(Platform::versionString()), allocator);
    doc.AddMember("algo",         rapidjson::StringRef(Options::i()->algoName()), allocator);
    doc.AddMember("uptime",       Workers::uptime(), allocator);
}


void ApiState::getResults(rapidjson::Document &doc) const
{
    auto &allocator = doc.GetAllocator();

    rapidjson::Value results(rapidjson::kObjectType);

    results.AddMember("diff_current",  m_network.diff, allocator);
    results.AddMember("shares_good",   m_network.accepted, allocator);
    results.AddMember("shares_total",  m_network.accepted + m_network.rejected, allocator);
    results.AddMember("avg_time",      m_network.avgTime(), allocator);
    results.AddMember("hashes_total",  m_network.total, allocator);

    rapidjson::Value best(rapidjson::kArrayType);
    for (size_t i = 0; i < m_network.topDiff.size(); ++i) {
        best.PushBack(m_network.topDiff[i], allocator);
    }

    results.AddMember("best",      best, allocator);
    results.AddMember("error_log", rapidjson::Value(rapidjson::kArrayType), allocator);

    doc.AddMember("results", results, allocator);
}