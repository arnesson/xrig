#include <stdio.h>
#include <string.h>

#ifdef __linux__
#define GetProcAddress dlsym
#endif

#include "adl/include/adl_sdk.h"
#include "adl/include/adl_structures.h"

#include <vector>
#include <algorithm>

#include "Adl.h"
#include "Options.h"
#include "log/Log.h"


// Memory allocation function
inline void* __stdcall ADL_Main_Memory_Alloc ( int iSize )
{
    void* lpBuffer = malloc ( iSize );
    return lpBuffer;
}

// Optional Memory de-allocation function
inline void __stdcall ADL_Main_Memory_Free ( void** lpBuffer )
{
    if ( NULL != *lpBuffer )
    {
        free ( *lpBuffer );
        *lpBuffer = NULL;
    }
}


Adl *Adl::m_self = nullptr;


void Adl::start(const std::vector<int> busIds) {
    m_self = new Adl(busIds);
}


Adl::Adl(const std::vector<int> busIds) {
	context = NULL;
	lpAdapterInfo = NULL;

	hDLL = NULL;
#ifdef _WIN32
	// Load the ADL dll
	hDLL = LoadLibrary(TEXT("atiadlxx.dll"));
	if (hDLL == NULL)
	{
		// A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
		// Try to load the 32 bit library (atiadlxy.dll) instead
		hDLL = LoadLibrary(TEXT("atiadlxy.dll"));
	}
	
	if (NULL == hDLL)
	{
		LOG_ERR("Failed to load ADL library\n");
	}

	ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL_Main_Control_Create");
	ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
	ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) GetProcAddress(hDLL,"ADL_Adapter_NumberOfAdapters_Get");
	ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET) GetProcAddress(hDLL,"ADL_Adapter_AdapterInfo_Get");
	ADL_AdapterX2_Caps = (ADL_ADAPTERX2_CAPS) GetProcAddress( hDLL, "ADL_AdapterX2_Caps");
	ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
	ADL2_OverdriveN_Capabilities_Get = (ADL2_OVERDRIVEN_CAPABILITIES_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_Capabilities_Get");
	ADL2_OverdriveN_CapabilitiesX2_Get = (ADL2_OVERDRIVEN_CAPABILITIESX2_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_CapabilitiesX2_Get");
	ADL2_OverdriveN_SystemClocks_Get = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_SystemClocks_Get");
	ADL2_OverdriveN_SystemClocksX2_Get = (ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_SystemClocksX2_Get");
	ADL2_OverdriveN_SystemClocks_Set = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_SystemClocks_Set");
	ADL2_OverdriveN_SystemClocksX2_Set = (ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_SystemClocksX2_Set");
	ADL2_OverdriveN_MemoryClocks_Get = (ADL2_OVERDRIVEN_MEMORYCLOCKS_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_MemoryClocks_Get");
	ADL2_OverdriveN_MemoryClocksX2_Get = (ADL2_OVERDRIVEN_MEMORYCLOCKSX2_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_MemoryClocksX2_Get");
	ADL2_OverdriveN_MemoryClocks_Set = (ADL2_OVERDRIVEN_MEMORYCLOCKS_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_MemoryClocks_Set");
	ADL2_OverdriveN_MemoryClocksX2_Set = (ADL2_OVERDRIVEN_MEMORYCLOCKSX2_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_MemoryClocksX2_Set");
	ADL2_OverdriveN_PerformanceStatus_Get = (ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) GetProcAddress (hDLL,"ADL2_OverdriveN_PerformanceStatus_Get");
	ADL2_OverdriveN_FanControl_Get = (ADL2_OVERDRIVEN_FANCONTROL_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_FanControl_Get");
	ADL2_OverdriveN_FanControl_Set = (ADL2_OVERDRIVEN_FANCONTROL_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_FanControl_Set");
	ADL2_OverdriveN_PowerLimit_Get = (ADL2_OVERDRIVEN_POWERLIMIT_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_PowerLimit_Get");
	ADL2_OverdriveN_PowerLimit_Set = (ADL2_OVERDRIVEN_POWERLIMIT_SET) GetProcAddress (hDLL, "ADL2_OverdriveN_PowerLimit_Set");
	ADL2_OverdriveN_Temperature_Get = (ADL2_OVERDRIVEN_TEMPERATURE_GET) GetProcAddress (hDLL, "ADL2_OverdriveN_Temperature_Get");
	ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS) GetProcAddress (hDLL, "ADL2_Overdrive_Caps");
	if ( NULL == ADL_Main_Control_Create ||
		 NULL == ADL_Main_Control_Destroy ||
		 NULL == ADL_Adapter_NumberOfAdapters_Get||
		 NULL == ADL_Adapter_AdapterInfo_Get ||
		 NULL == ADL_AdapterX2_Caps ||
		NULL == ADL2_Adapter_Active_Get ||
		NULL == ADL2_OverdriveN_Capabilities_Get || 
		NULL == ADL2_OverdriveN_CapabilitiesX2_Get || 
		NULL == ADL2_OverdriveN_SystemClocks_Get ||
		NULL == ADL2_OverdriveN_SystemClocksX2_Get ||
		NULL == ADL2_OverdriveN_SystemClocks_Set ||
		NULL == ADL2_OverdriveN_SystemClocksX2_Set ||
		NULL == ADL2_OverdriveN_MemoryClocks_Get ||
		NULL == ADL2_OverdriveN_MemoryClocksX2_Get ||
		NULL == ADL2_OverdriveN_MemoryClocks_Set ||
		NULL == ADL2_OverdriveN_MemoryClocksX2_Set ||
		NULL == ADL2_OverdriveN_PerformanceStatus_Get ||
		NULL == ADL2_OverdriveN_FanControl_Get ||
		NULL == ADL2_OverdriveN_FanControl_Set ||
		NULL == ADL2_Overdrive_Caps
		)
	{
		LOG_ERR("Failed to get ADL function pointers");
	}
#endif // _WIN32
#ifdef __linux__
	hDLL = dlopen( "libatiadlxx.so", RTLD_LAZY|RTLD_GLOBAL);
	if (NULL == hDLL)
	{
            LOG_ERR("ADL library not found!");
	    exit(1);
	}

	ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE) GetProcAddress(hDLL,"ADL_Main_Control_Create");
	ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY) GetProcAddress(hDLL,"ADL_Main_Control_Destroy");
	ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET) GetProcAddress(hDLL,"ADL_Adapter_NumberOfAdapters_Get");
	ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET) GetProcAddress(hDLL,"ADL_Adapter_AdapterInfo_Get");
#endif // __linux__

	if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
	{
		LOG_ERR("ADL_Main_Control_Create failed");
	}

	int iNumberAdapters = 0;

	// Obtain the number of adapters for the system
	if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters))
	{
		LOG_ERR("ADL_Adapter_NumberOfAdapters_Get failed");
	}
	
	if (0 < iNumberAdapters)
	{
		lpAdapterInfo = (LPAdapterInfo)malloc(sizeof (AdapterInfo)* iNumberAdapters);
		memset(lpAdapterInfo, '\0', sizeof (AdapterInfo)* iNumberAdapters);

		// Get the AdapterInfo structure for all adapters in the system
		ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof (AdapterInfo)* iNumberAdapters);
	}

	for (int i = 0; i < iNumberAdapters; i++) {
		if (std::find(busIds.begin(), busIds.end(), lpAdapterInfo[i].iBusNumber) != busIds.end()) {
	    	adapters[lpAdapterInfo[i].iBusNumber] = lpAdapterInfo[i].iAdapterIndex;
		}
	}

	for (const auto& map : adapters) {
    	int busId = map.first;
    	int adapterIndex = map.second;

		systemClocks[busId] = OverdriveN_SystemClocksX2_Get(adapterIndex);
		memoryClocks[busId] = OverdriveN_MemoryClocksX2_Get(adapterIndex);
		fanControl[busId] = OverdriveN_FanControl_Get(adapterIndex);
		performanceStatus[busId] = OverdriveN_PerformanceStatus_Get(adapterIndex);
		temperature[busId] = OverdriveN_Temperature_Get(adapterIndex);
		powerLimit[busId] = OverdriveN_PowerLimit_Get(adapterIndex);

		// fix underclocking for vega cards
		if (systemClocks[busId]->aLevels[7].iVddc > 1000) {
			setSystemClock(adapterIndex, 7, systemClocks[busId]->aLevels[7].iClock, systemClocks[busId]->aLevels[7].iVddc - 1);
		} else {
			setSystemClock(adapterIndex, 7, systemClocks[busId]->aLevels[7].iClock, systemClocks[busId]->aLevels[7].iVddc + 1);
		}
		setSystemClock(adapterIndex, 7, systemClocks[busId]->aLevels[7].iClock, systemClocks[busId]->aLevels[7].iVddc);

		// apply profile
		const std::vector<ADLODNPerformanceLevelX2*> &odNSystemClocks = Options::i()->systemClocks();
		const std::vector<ADLODNPerformanceLevelX2*> &odNMemoryClocks = Options::i()->memoryClocks();

		for (auto i=0; i < odNSystemClocks.size(); i++) {
		    setSystemClock(adapterIndex, i, odNSystemClocks[i]->iClock, odNSystemClocks[i]->iVddc);
		}
		for (auto i=0; i < odNMemoryClocks.size(); i++) {
		    setMemoryClock(adapterIndex, i, odNMemoryClocks[i]->iClock, odNMemoryClocks[i]->iVddc);
		}
		if (Options::i()->targetTemperature()) {
			setTargetTemperature(adapterIndex, Options::i()->targetTemperature());
		}
		if (Options::i()->minFanLimit()) {
			setMinFanLimit(adapterIndex, Options::i()->minFanLimit());
		}
		if (Options::i()->powerLimit() != 0) {
			setPowerLimit(adapterIndex, Options::i()->powerLimit());
		}
	}
}

Adl::~Adl()
{
	ADL_Main_Memory_Free((void**)&lpAdapterInfo);

	ADL_Main_Control_Destroy();
	
#ifdef _WIN32
	FreeLibrary(hDLL);
#endif // _WIN32
}


ADLODNPerformanceLevelsX2* Adl::OverdriveN_SystemClocksX2_Get(int adapterIndex) {
	ADLODNPerformanceLevelsX2 *odNPerformanceLevels;

	int size = sizeof(ADLODNPerformanceLevelsX2) + sizeof(ADLODNPerformanceLevelX2)* (ADL_PERFORMANCE_LEVELS - 1);
	void* odNPerformanceLevelsBuffer = new char[size];
	memset(odNPerformanceLevelsBuffer, 0, size);
	odNPerformanceLevels = (ADLODNPerformanceLevelsX2*)odNPerformanceLevelsBuffer;
	odNPerformanceLevels->iSize = size;
	odNPerformanceLevels->iNumberOfPerformanceLevels = ADL_PERFORMANCE_LEVELS;

	if (ADL_OK != ADL2_OverdriveN_SystemClocksX2_Get(context, adapterIndex, odNPerformanceLevels)) {
		LOG_ERR("ADL2_OverdriveN_SystemClocksX2_Get failed. adapterIndex: %d", adapterIndex);
	}

	return odNPerformanceLevels;
}

ADLODNPerformanceLevelsX2* Adl::OverdriveN_MemoryClocksX2_Get(int adapterIndex) {
	ADLODNPerformanceLevelsX2 *odNPerformanceLevels;

	int size = sizeof(ADLODNPerformanceLevelsX2) + sizeof(ADLODNPerformanceLevelX2)* (ADL_PERFORMANCE_LEVELS - 1);
	void* odNPerformanceLevelsBuffer = new char[size];
	memset(odNPerformanceLevelsBuffer, 0, size);
	odNPerformanceLevels = (ADLODNPerformanceLevelsX2*)odNPerformanceLevelsBuffer;
	odNPerformanceLevels->iSize = size;
	odNPerformanceLevels->iNumberOfPerformanceLevels = ADL_PERFORMANCE_LEVELS;

	if (ADL_OK != ADL2_OverdriveN_MemoryClocksX2_Get(context, adapterIndex, odNPerformanceLevels)) {
		LOG_ERR("ADL2_OverdriveN_MemoryClocksX2_Get failed. adapterIndex: %d", adapterIndex);
	}

	return odNPerformanceLevels;
}

ADLODNFanControl* Adl::OverdriveN_FanControl_Get(int adapterIndex) {
	ADLODNFanControl *odNFanControl;
	
	int size = sizeof(ADLODNFanControl);
	void* odNFanControlBuffer = new char[size];
	memset(odNFanControlBuffer, 0, size);
	odNFanControl = (ADLODNFanControl*)odNFanControlBuffer;

	if (ADL_OK != ADL2_OverdriveN_FanControl_Get(context, adapterIndex, odNFanControl))
	{
		LOG_ERR("ADL2_OverdriveN_FanControl_Get failed. adapterIndex: %d", adapterIndex);
	}

	return odNFanControl;
}

ADLODNPerformanceStatus* Adl::OverdriveN_PerformanceStatus_Get(int adapterIndex) {
	ADLODNPerformanceStatus *odNPerformanceStatus;

	int size = sizeof(ADLODNPerformanceStatus);
	void* odNPerformanceStatusBuffer = new char[size];
	memset(odNPerformanceStatusBuffer, 0, size);
	odNPerformanceStatus = (ADLODNPerformanceStatus*)odNPerformanceStatusBuffer;

	if (ADL_OK != ADL2_OverdriveN_PerformanceStatus_Get(context, adapterIndex, odNPerformanceStatus))
	{
		LOG_ERR("ADL2_OverdriveN_PerformanceStatus_Get failed. adapterIndex: %d", adapterIndex);
	}

	return odNPerformanceStatus;
}

ADLODNPowerLimitSetting* Adl::OverdriveN_PowerLimit_Get(int adapterIndex) {
	ADLODNPowerLimitSetting *odNPowerControl;

	int size = sizeof(ADLODNPowerLimitSetting);
	void* odNPowerControlBuffer = new char[size];
	memset(odNPowerControlBuffer, 0, size);
	odNPowerControl = (ADLODNPowerLimitSetting*)odNPowerControlBuffer;

	if (ADL_OK != ADL2_OverdriveN_PowerLimit_Get(context, adapterIndex, odNPowerControl)) {
		LOG_ERR("ADL2_OverdriveN_PowerLimit_Get failed. adapterIndex: %d", adapterIndex);
	}

	return odNPowerControl;
}

int Adl::OverdriveN_Temperature_Get(int adapterIndex) {
	int temp = 0;

	if (ADL_OK != ADL2_OverdriveN_Temperature_Get(context, adapterIndex, 1, &temp)) {
		LOG_ERR("ADL2_OverdriveN_Temperature_Get failed. adapterIndex: %d", adapterIndex);
	}

	return temp;
}

void Adl::setSystemClock(int adapterIndex, int level, int clock, int vddc) {
	ADLODNPerformanceLevelsX2* odNPerformanceLevels = OverdriveN_SystemClocksX2_Get(adapterIndex);

	if (level > ADL_PERFORMANCE_LEVELS || level < 0) {
		LOG_ERR("Invalid performance level: %d", level);			
	} else if (odNPerformanceLevels->aLevels[level].iEnabled == 1) {
		odNPerformanceLevels->aLevels[level].iClock = clock;
		odNPerformanceLevels->aLevels[level].iVddc = vddc;
		odNPerformanceLevels->iMode = ADLODNControlType::ODNControlType_Manual;

		if (ADL_OK != ADL2_OverdriveN_SystemClocksX2_Set(context, adapterIndex, odNPerformanceLevels)) {
			LOG_ERR("ADL2_OverdriveN_SystemClocksX2_Set failed. adapterIndex: %d", adapterIndex);
		}
	}
}

void Adl::setMemoryClock(int adapterIndex, int level, int clock, int vddc) {
	ADLODNPerformanceLevelsX2* odNPerformanceLevels = OverdriveN_MemoryClocksX2_Get(adapterIndex);

	if (level > ADL_PERFORMANCE_LEVELS || level < 0) {
		LOG_ERR("Invalid performance level: %d", level);			
	} else if (odNPerformanceLevels->aLevels[level].iEnabled == 1) {
		odNPerformanceLevels->aLevels[level].iClock = clock;
		odNPerformanceLevels->aLevels[level].iVddc = vddc;
		odNPerformanceLevels->iMode = ADLODNControlType::ODNControlType_Manual;

		if (ADL_OK != ADL2_OverdriveN_MemoryClocksX2_Set(context, adapterIndex, odNPerformanceLevels)) {
			LOG_ERR("ADL2_OverdriveN_MemoryClocksX2_Set failed. adapterIndex: %d", adapterIndex);
		}
	}
}

void Adl::setPowerLimit(int adapterIndex, int powerLimit) {
	ADLODNPowerLimitSetting* odNPowerControl = OverdriveN_PowerLimit_Get(adapterIndex);

	odNPowerControl->iTDPLimit = powerLimit;
	odNPowerControl->iMode = ADLODNControlType::ODNControlType_Manual;

	if (ADL_OK != ADL2_OverdriveN_PowerLimit_Set(context, adapterIndex, odNPowerControl)) {
		LOG_ERR("ADL2_OverdriveN_PowerLimit_Set failed. adapterIndex: %d", adapterIndex);
	}
}

void Adl::setTargetTemperature(int adapterIndex, int targetTemperature) {
	ADLODNFanControl* odNFanControl = OverdriveN_FanControl_Get(adapterIndex);

	odNFanControl->iTargetTemperature = targetTemperature;
	odNFanControl->iMode = ADLODNControlType::ODNControlType_Manual;

	if (ADL_OK != ADL2_OverdriveN_FanControl_Set(context, adapterIndex, odNFanControl)) {
		LOG_ERR("ADL2_OverdriveN_FanControl_Set failed. adapterIndex: %d", adapterIndex);
	}
}

void Adl::setMinFanLimit(int adapterIndex, int minFanLimit) {
	ADLODNFanControl* odNFanControl = OverdriveN_FanControl_Get(adapterIndex);

	odNFanControl->iMinFanLimit = minFanLimit;
	odNFanControl->iMode = ADLODNControlType::ODNControlType_Manual;

	if (ADL_OK != ADL2_OverdriveN_FanControl_Set(context, adapterIndex, odNFanControl)) {
		LOG_ERR("ADL2_OverdriveN_FanControl_Set failed. adapterIndex: %d", adapterIndex);
	}
}

void Adl::tick() {
	for (const auto& map : adapters) {
    	int busId = map.first;
    	int adapterIndex = map.second;

		systemClocks[busId] = OverdriveN_SystemClocksX2_Get(adapterIndex);
		memoryClocks[busId] = OverdriveN_MemoryClocksX2_Get(adapterIndex);
		fanControl[busId] = OverdriveN_FanControl_Get(adapterIndex);
		performanceStatus[busId] = OverdriveN_PerformanceStatus_Get(adapterIndex);
		temperature[busId] = OverdriveN_Temperature_Get(adapterIndex);
		powerLimit[busId] = OverdriveN_PowerLimit_Get(adapterIndex);
	}
}
