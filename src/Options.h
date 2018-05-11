/* XMRig
 * Copyright 2010      Jeff Garzik <jgarzik@pobox.com>
 * Copyright 2012-2014 pooler      <pooler@litecoinpool.org>
 * Copyright 2014      Lucas Jones <https://github.com/lucasjones>
 * Copyright 2014-2016 Wolf9466    <https://github.com/OhGodAPet>
 * Copyright 2016      Jay D Dee   <jayddee246@gmail.com>
 * Copyright 2017-2018 XMR-Stak    <https://github.com/fireice-uk>, <https://github.com/psychocrypt>
 * Copyright 2016-2018 XMRig       <https://github.com/xmrig>, <support@xmrig.com>

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

#ifndef __OPTIONS_H__
#define __OPTIONS_H__


#include <stdint.h>
#include <vector>


#include "amd/Adl.h"
#include "amd/OclCLI.h"
#include "rapidjson/fwd.h"


class OclThread;
class Url;
struct option;

class Options
{
public:
    enum Algo {
        ALGO_CRYPTONIGHT,      /* CryptoNight (Monero) */
        ALGO_CRYPTONIGHT_LITE, /* CryptoNight-Lite (AEON) */
    };

    enum Variant {
        VARIANT_AUTO = -1,
        VARIANT_NONE = 0,
        VARIANT_V1   = 1
    };

    enum AlgoVariant {
        AV0_AUTO,
        AV1_AESNI,
        AV2_UNUSED,
        AV3_SOFT_AES,
        AV4_UNUSED,
        AV_MAX
    };

    static inline Options* i() { return m_self; }
    static Options *parse(int argc, char **argv);

    inline bool background() const                        { return m_background; }
    inline bool colors() const                            { return m_colors; }
    inline bool syslog() const                            { return m_syslog; }
    inline const char *accessToken() const                { return m_accessToken; }
    inline const char *id() const                         { return m_id; }
    inline const char *log() const                        { return m_log; }
    inline const std::vector<OclThread*> &threads() const { return m_threads; }
    inline const std::vector<Url*> &pools() const         { return m_pools; }
    inline int algo() const                               { return m_algo; }
    inline int algoVariant() const                        { return m_algoVariant; }
    inline int port() const                               { return m_port; }
    inline size_t intensity() const                       { return m_intensity; }
    inline int platformIndex() const                      { return m_platformIndex; }
    inline int printTime() const                          { return 60; }
    inline int retries() const                            { return 5; }
    inline int retryPause() const                         { return 5; }
    inline void setColors(bool colors)                    { m_colors = colors; }
    inline const std::vector<ADLODNPerformanceLevelX2*> &systemClocks() const { return m_systemClocks; }
    inline const std::vector<ADLODNPerformanceLevelX2*> &memoryClocks() const { return m_memoryClocks; }
    inline int targetTemperature() const                  { return m_targetTemperature; }
    inline int powerLimit() const                         { return m_powerLimit; }
    inline int minFanLimit() const                        { return m_minFanLimit; }

    inline static void release()                          { delete m_self; }

    bool oclInit();

    const char *algoName() const;

private:
    Options(int argc, char **argv);
    ~Options();

    inline bool isReady() const { return m_ready; }

    static Options *m_self;

    bool getJSON(const char *fileName, rapidjson::Document &doc);
    bool parseArg(int key, const char *arg);
    bool parseArg(int key, uint64_t arg);
	bool parseArg(int key, int64_t arg);
    bool parseBoolean(int key, bool enable);
    Url *parseUrl(const char *arg) const;
    void parseConfig(const char *fileName);
    void parseProfile(const struct option *option, const rapidjson::Value &object);
    void parseJSON(const struct option *option, const rapidjson::Value &object);
    void parseLevel(int key, const rapidjson::Value &object);
    void parseThread(const rapidjson::Value &object);
    void showUsage(int status) const;
    void showVersion(void);

    bool setAlgo(const char *algo);

    bool m_background;
    bool m_colors;
    bool m_ready;
    bool m_syslog;
    char *m_accessToken;
    char *m_id;
    char *m_config;
    char *m_log;
    int m_algo;
    int m_algoVariant;
    int m_port;
    size_t m_intensity;
    int m_platformIndex;
    OclCLI m_oclCLI;
    std::vector<OclThread*> m_threads;
    std::vector<Url*> m_pools;
    std::vector<ADLODNPerformanceLevelX2*> m_systemClocks;
    std::vector<ADLODNPerformanceLevelX2*> m_memoryClocks;
    int m_targetTemperature;
    int m_powerLimit;
    int m_minFanLimit;
};

#endif /* __OPTIONS_H__ */
