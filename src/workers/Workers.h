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

#ifndef __WORKERS_H__
#define __WORKERS_H__


#include <atomic>
#include <list>
#include <uv.h>
#include <vector>
#include <map>

#include "net/Job.h"
#include "net/JobResult.h"


class Handle;
class Hashrate;
class IJobResultListener;


class Workers
{
public:
    static bool start(const std::vector<OclThread*> &threads);
    static Job job();
    static void printHashrate(bool detail);
    static void setEnabled(bool enabled);
    static void setJob(const Job &job, bool donate);
    static void stop();
    static void submit(const Job &result);
    static bool isDead(int threadId);

    static inline bool isEnabled()                               { return m_enabled; }
    static inline bool isOutdated(uint64_t sequence)             { return m_sequence.load(std::memory_order_relaxed) != sequence; }
    static inline bool isPaused()                                { return m_paused.load(std::memory_order_relaxed) == 1; }
    static inline uint64_t sequence()                            { return m_sequence.load(std::memory_order_relaxed); }
    static inline void pause()                                   { m_active = false; m_paused = 1; m_sequence++; }
    static inline void setListener(IJobResultListener *listener) { m_listener = listener; }
    static inline uint64_t uptime()                              { return (uv_now(uv_default_loop()) - m_start) / 1000; }

    static const std::vector<Handle*> &handles()                 { return m_handles; }
    static const std::map<int, std::vector<Handle*>> &gpus()     { return m_gpus; }

private:
    static void onReady(void *arg);
    static void onResult(uv_async_t *handle);
    static void onReport(uv_timer_t *handle);
    static void onTick(uv_timer_t *handle);

    static bool m_active;
    static bool m_enabled;
    static Hashrate *m_hashrate;
    static IJobResultListener *m_listener;
    static Job m_job;
    static std::atomic<int> m_paused;
    static std::atomic<uint64_t> m_sequence;
    static std::list<Job> m_queue;
    static std::vector<Handle*> m_handles;
    static std::map<int, std::vector<Handle*>> m_gpus;
    static uint64_t m_ticks;
    static uv_async_t m_async;
    static uv_mutex_t m_mutex;
    static uv_rwlock_t m_rwlock;
    static uv_timer_t m_reportTimer;
    static uv_timer_t m_timer;
    static uint64_t m_start;
};


#endif /* __WORKERS_H__ */