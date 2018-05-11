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


#include <string.h>
#include <uv.h>


#ifdef _MSC_VER
#   include "getopt/getopt.h"
#else
#   include <getopt.h>
#endif

#ifndef XMRIG_NO_HTTPD
#   include <microhttpd.h>
#endif


#include "amd/Adl.h"
#include "amd/OclGPU.h"
#include "Cpu.h"
#include "log/Log.h"
#include "net/Url.h"
#include "Options.h"
#include "Platform.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/prettywriter.h"
#include "version.h"
#include "workers/OclThread.h"


#ifndef ARRAY_SIZE
#   define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif


Options *Options::m_self = nullptr;


static char const usage[] = "\
Usage: " APP_ID " [OPTIONS]\n\
\n\
Options:\n\
  -a, --algo=ALGO           cryptonight (default) or cryptonight-lite\n\
  -o, --url=URL             URL of mining server\n\
  -u, --user=USERNAME       username for mining server\n\
  -p, --pass=PASSWORD       password for mining server\n\
      --variant=N           PoW variant (defaults to -1 = automatic)\n\
  -k, --keepalive           send keepalive to prevent timeout (needs pool support)\n\
      --intensity=N         thread intensity\n\
      --platform-index=N    OpenCL platform index\n\
  -b, --background          run the miner in the background\n\
  -c, --config=FILE         load a JSON-format configuration file\n\
  -l, --log=FILE            log all output to a file\n"
# ifdef HAVE_SYSLOG_H
"\
  -s, --syslog              use system log for output messages\n"
# endif
"\
      --nicehash            enable nicehash support\n\
      --port=N              port for the miner API\n\
      --token=T             access token for API\n\
      --id=ID               miner id (defaults to machine name)\n\
  -h, --help                display this help and exit\n\
  -v, --version             output version information and exit\n\
";


static char const short_options[] = "a:c:khbp:o:u:vl:s";


static struct option const options[] = {
    { "algo",             1, nullptr, 'a'  },
    { "token",            1, nullptr, 4001 },
    { "port",             1, nullptr, 4000 },
    { "id",               1, nullptr, 4002 },
    { "background",       0, nullptr, 'b'  },
    { "config",           1, nullptr, 'c'  },
    { "help",             0, nullptr, 'h'  },
    { "keepalive",        0, nullptr ,'k'  },
    { "log",              1, nullptr, 'l'  },
    { "nicehash",         0, nullptr, 1006 },
    { "no-color",         0, nullptr, 1002 },
    { "variant",          1, nullptr, 1010 },
    { "intensity",        1, nullptr, 1401 },
    { "platform-index",   1, nullptr, 1400 },
    { "pass",             1, nullptr, 'p'  },
    { "syslog",           0, nullptr, 's'  },
    { "url",              1, nullptr, 'o'  },
    { "user",             1, nullptr, 'u'  },
    { "version",          0, nullptr, 'v'  },
    { 0, 0, 0, 0 }
};


static struct option const config_options[] = {
    { "algo",             1, nullptr, 'a'  },
    { "background",       0, nullptr, 'b'  },
    { "log",              1, nullptr, 'l'  },
    { "platform_index",   1, nullptr, 1400 },
    { "intensity",        1, nullptr, 1401 },
    { "syslog",           0, nullptr, 's'  },
    { 0, 0, 0, 0 }
};


static struct option const pool_options[] = {
    { "url",           1, nullptr, 'o'  },
    { "pass",          1, nullptr, 'p'  },
    { "user",          1, nullptr, 'u'  },
    { "keepalive",     0, nullptr ,'k'  },
    { "nicehash",      0, nullptr, 1006 },
    { "variant",       1, nullptr, 1010 },
    { 0, 0, 0, 0 }
};


static struct option const api_options[] = {
    { "port",          1, nullptr, 4000 },
    { "token",         1, nullptr, 4001 },
    { "id",            1, nullptr, 4002 },
    { 0, 0, 0, 0 }
};

static struct option const profile_options[] = {
    { "system",               1, nullptr, 5000 },
    { "memory",               1, nullptr, 5001 },
    { "target_temperature",   1, nullptr, 5002 },
    { "power_limit",          1, nullptr, 5003 },
    { "min_fan_limit",        1, nullptr, 5004 },
    { 0, 0, 0, 0 }
};


static const char *algo_names[] = {
    "cryptonight",
#   ifndef XMRIG_NO_AEON
    "cryptonight-lite"
#   endif
};


Options *Options::parse(int argc, char **argv)
{
    Options *options = new Options(argc, argv);
    if (options->isReady()) {
        m_self = options;
        return m_self;
    }

    delete options;
    return nullptr;
}


bool Options::oclInit()
{
    if (m_threads.empty() && !m_oclCLI.setup(m_threads)) {
        m_oclCLI.autoConf(m_threads, &m_platformIndex, &m_intensity);
    }

    return true;
}


const char *Options::algoName() const
{
    return algo_names[m_algo];
}


Options::Options(int argc, char **argv) :
    m_background(false),
    m_colors(true),
    m_ready(false),
    m_syslog(false),
    m_accessToken(nullptr),
    m_id(nullptr),
    m_config(nullptr),
    m_log(nullptr),
    m_algo(0),
    m_algoVariant(0),
    m_port(0),
    m_intensity(0),
    m_platformIndex(0),
    m_threads(0),
    m_systemClocks(0),
    m_memoryClocks(0),
    m_targetTemperature(0),
    m_powerLimit(0),
    m_minFanLimit(0)
{
    m_pools.push_back(new Url());

    int key;

    while (1) {
        key = getopt_long(argc, argv, short_options, options, NULL);
        if (key < 0) {
            break;
        }

        if (!parseArg(key, optarg)) {
            return;
        }
    }

    if (optind < argc) {
        fprintf(stderr, "%s: unsupported non-option argument '%s'\n", argv[0], argv[optind]);
        return;
    }

    if (!m_pools[0]->isValid()) {
        parseConfig(Platform::defaultConfig());
    }

    if (!m_pools[0]->isValid()) {
        Url *url = new Url("cryptonight.eu.nicehash.com", 3355, "3EXnQ9TLnco6hqjL8S7685YF7mgkaN4LFq", nullptr, false, true);
        m_pools[0] = url;
    }

    m_algoVariant = Cpu::hasAES() ? AV1_AESNI : AV3_SOFT_AES;

    for (Url *url : m_pools) {
        url->setAlgo(m_algo);
        url->applyExceptions();
    }

    m_ready = true;
}


Options::~Options()
{
}


bool Options::getJSON(const char *fileName, rapidjson::Document &doc)
{
    uv_fs_t req;
    const int fd = uv_fs_open(uv_default_loop(), &req, fileName, O_RDONLY, 0644, nullptr);
    if (fd < 0) {
        if (fileName != Platform::defaultConfig()) {
            fprintf(stderr, "unable to open %s: %s\n", fileName, uv_strerror(fd));
        }
        return false;
    }

    uv_fs_req_cleanup(&req);

    FILE *fp = fdopen(fd, "rb");
    char buf[8192];
    rapidjson::FileReadStream is(fp, buf, sizeof(buf));

    doc.ParseStream(is);

    uv_fs_close(uv_default_loop(), &req, fd, nullptr);
    uv_fs_req_cleanup(&req);

    if (doc.HasParseError()) {
        printf("%s:%d: %s\n", fileName, (int)doc.GetErrorOffset(), rapidjson::GetParseError_En(doc.GetParseError()));
        return false;
    }

    return doc.IsObject();
}


bool Options::parseArg(int key, const char *arg)
{
    switch (key) {
    case 'a': /* --algo */
        if (!setAlgo(arg)) {
            return false;
        }
        break;

    case 'o': /* --url */
        if (m_pools.size() > 1 || m_pools[0]->isValid()) {
            Url *url = new Url(arg);
            if (url->isValid()) {
                m_pools.push_back(url);
            }
            else {
                delete url;
            }
        }
        else {
            m_pools[0]->parse(arg);
        }

        if (!m_pools.back()->isValid()) {
            return false;
        }
        break;

    case 'u': /* --user */
        m_pools.back()->setUser(arg);
        break;

    case 'p': /* --pass */
        m_pools.back()->setPassword(arg);
        break;

    case 'l': /* --log */
        free(m_log);
        m_log = strdup(arg);
        break;

    case 4001: /* --token */
        free(m_accessToken);
        m_accessToken = strdup(arg);
        break;

    case 4002: /* --id */
        free(m_id);
        m_id = strdup(arg);
        break;

    case 4000: /* --port */
    case 1400: /* --platform-index */
    case 1401: /* --intensity */
        return parseArg(key, (uint64_t) strtol(arg, nullptr, 10));

    case 1010: /* --variant */
        return parseArg(key, (int64_t) strtol(arg, nullptr, 10));

    case 'b':  /* --background */
    case 'k':  /* --keepalive */
    case 's':  /* --syslog */
    case 1006: /* --nicehash */
        return parseBoolean(key, true);

    case 'v': /* --version */
        showVersion();
        return false;

    case 'h': /* --help */
        showUsage(0);
        return false;

    case 'c': /* --config */
        parseConfig(arg);
        break;

    default:
        showUsage(1);
        return false;
    }

    return true;
}


bool Options::parseArg(int key, uint64_t arg)
{
    switch (key) {
    case 1010: /* --variant */
        m_pools.back()->setVariant((int) arg);
        break;

    case 1400: /* --platform-index */
        m_platformIndex = (int) arg;
        break;

    case 1401: /* --intensity */
        m_intensity = (size_t) arg;
        break;

    case 4000: /* --port */
        if (arg <= 65536) {
            m_port = (int) arg;
        }
        break;

    case 5002: /* profile.target_temperature */
        m_targetTemperature = (int) arg;
        break;

    case 5004: /* profile.min_fan_limit */
        m_minFanLimit = (int) arg;
        break;

    default:
        break;
    }

    return true;
}


bool Options::parseArg(int key, int64_t arg)
{
	switch (key) {
	case 5003: /* profile.power_limit */
		m_powerLimit = (int)arg;
		break;
		
	default:
		break;
	}

	return true;
}


bool Options::parseBoolean(int key, bool enable)
{
    switch (key) {
    case 'k': /* --keepalive */
        m_pools.back()->setKeepAlive(enable);
        break;

    case 'b': /* --background */
        m_background = enable;
        break;

    case 's': /* --syslog */
        m_syslog = enable;
        break;

    case 1006: /* --nicehash */
        m_pools.back()->setNicehash(enable);
        break;

    default:
        break;
    }

    return true;
}


Url *Options::parseUrl(const char *arg) const
{
    auto url = new Url(arg);
    if (!url->isValid()) {
        delete url;
        return nullptr;
    }

    return url;
}


void Options::parseConfig(const char *fileName)
{
    rapidjson::Document doc;
    if (!getJSON(fileName, doc)) {
        return;
    }

    m_config = strdup(fileName);

    for (size_t i = 0; i < ARRAY_SIZE(config_options); i++) {
        parseJSON(&config_options[i], doc);
    }

    const rapidjson::Value &pools = doc["pools"];
    if (pools.IsArray()) {
        for (const rapidjson::Value &value : pools.GetArray()) {
            if (!value.IsObject()) {
                continue;
            }

            for (size_t i = 0; i < ARRAY_SIZE(pool_options); i++) {
                parseJSON(&pool_options[i], value);
            }
        }
    }

    const rapidjson::Value &threads = doc["threads"];
    if (threads.IsArray()) {
        for (const rapidjson::Value &value : threads.GetArray()) {
            if (!value.IsObject()) {
                continue;
            }

            parseThread(value);
        }
    }

    const rapidjson::Value &api = doc["api"];
    if (api.IsObject()) {
        for (size_t i = 0; i < ARRAY_SIZE(api_options); i++) {
            parseJSON(&api_options[i], api);
        }
    }

    const rapidjson::Value &profile = doc["profile"];
    if (profile.IsObject()) {
        for (size_t i = 0; i < ARRAY_SIZE(profile_options); i++) {
            parseProfile(&profile_options[i], profile);
        }
    }
}


void Options::parseProfile(const struct option *option, const rapidjson::Value &object)
{
    if (!option->name || !object.HasMember(option->name)) {
        return;
    }

    const rapidjson::Value &value = object[option->name];

    if (option->has_arg && value.IsArray()) {
        for (const rapidjson::Value &level : value.GetArray()) {
            if (!level.IsObject()) {
                continue;
            }

            parseLevel(option->val, level);
        }
    }
    else if (option->has_arg && value.IsString()) {
        parseArg(option->val, value.GetString());
    }
    else if (option->has_arg && value.IsUint64()) {
        parseArg(option->val, value.GetUint64());
	}
	else if (option->has_arg && value.IsInt64()) {
		parseArg(option->val, value.GetInt64());
	}
    else if (!option->has_arg && value.IsBool()) {
        parseBoolean(option->val, value.IsTrue());
    }
}


void Options::parseJSON(const struct option *option, const rapidjson::Value &object)
{
    if (!option->name || !object.HasMember(option->name)) {
        return;
    }

    const rapidjson::Value &value = object[option->name];

    if (option->has_arg && value.IsString()) {
        parseArg(option->val, value.GetString());
    }
    else if (option->has_arg && value.IsUint64()) {
        parseArg(option->val, value.GetUint64());
    }
    else if (option->has_arg && value.IsInt64()) {
        parseArg(option->val, value.GetInt64());
    }
    else if (!option->has_arg && value.IsBool()) {
        parseBoolean(option->val, value.IsTrue());
    }
}


void Options::parseLevel(int key, const rapidjson::Value &object)
{
    ADLODNPerformanceLevelX2 *odNPerformanceLevel = new ADLODNPerformanceLevelX2();

    const rapidjson::Value &clock = object["clock"];
    if (clock.IsInt()) {
        odNPerformanceLevel->iClock = clock.GetInt() * 100;
    }

    const rapidjson::Value &vddc = object["vddc"];
    if (vddc.IsDouble()) {
        odNPerformanceLevel->iVddc = (int) (vddc.GetDouble() * 1000);
    }

    switch (key) {
    case 5000: /* profile.system */
        m_systemClocks.push_back(odNPerformanceLevel);
        break;

    case 5001: /* profile.memory */
        m_memoryClocks.push_back(odNPerformanceLevel);
        break;

    default:
        break;
    }
}


void Options::parseThread(const rapidjson::Value &object)
{
    OclThread *thread = new OclThread();
    thread->setIndex(object["index"].GetInt());

	const rapidjson::Value &intensity = object["intensity"];
	if (intensity.IsUint()) {
		thread->setRawIntensity(intensity.GetUint());
	}

	const rapidjson::Value &worksize = object["worksize"];
	if (worksize.IsUint()) {
		thread->setWorksize(worksize.GetUint());
	}

    const rapidjson::Value &affinity = object["affine_to_cpu"];
    if (affinity.IsInt()) {
        thread->setAffinity(affinity.GetInt());
    }

    m_threads.push_back(thread);
}


void Options::showUsage(int status) const
{
    if (status) {
        fprintf(stderr, "Try \"" APP_ID "\" --help' for more information.\n");
    }
    else {
        printf(usage);
    }
}


void Options::showVersion()
{
    printf("%s\n", Platform::versionString());
}


bool Options::setAlgo(const char *algo)
{
    for (size_t i = 0; i < ARRAY_SIZE(algo_names); i++) {
        if (algo_names[i] && !strcmp(algo, algo_names[i])) {
            m_algo = (int) i;
            break;
        }

#       ifndef XMRIG_NO_AEON
        if (i == ARRAY_SIZE(algo_names) - 1 && !strcmp(algo, "cryptonight-light")) {
            m_algo = ALGO_CRYPTONIGHT_LITE;
            break;
        }
#       endif

        if (i == ARRAY_SIZE(algo_names) - 1) {
            showUsage(1);
            return false;
        }
    }

    return true;
}
