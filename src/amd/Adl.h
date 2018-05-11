#ifndef __ADL_H__
#define __ADL_H__

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif // _WIN32

#include <vector>
#include <map>


#include "adl/include/adl_sdk.h"
#include "adl/include/adl_structures.h"


#define ADL_PERFORMANCE_LEVELS 8 


class Adl
{
public:
	static inline Adl* i() { return m_self; }
	static void start(const std::vector<int> busIds);

	void tick();
	inline const ADLODNPerformanceLevelsX2* getSystemClocks(int busId) { return systemClocks[busId]; };
	inline const ADLODNPerformanceLevelsX2* getMemoryClocks(int busId) { return memoryClocks[busId]; };
	inline const ADLODNFanControl* getFanControl(int busId) { return fanControl[busId]; };
	inline const ADLODNPerformanceStatus* getPerformanceStatus(int busId) { return performanceStatus[busId]; };
	inline const ADLODNPowerLimitSetting* getPowerLimit(int busId) { return powerLimit[busId]; };
	inline int getTemperature(int busId) { return temperature[busId]; };
	inline int getAdapterIndex(int busId) { return adapters[busId]; };

private:
	Adl(const std::vector<int> busIds);
	~Adl();

	static Adl *m_self;

	Adl(Adl const&);
    void operator=(Adl const&);

	typedef int ( *ADL_MAIN_CONTROL_CREATE )(ADL_MAIN_MALLOC_CALLBACK, int );
	typedef int ( *ADL_MAIN_CONTROL_DESTROY )();
	typedef int ( *ADL_FLUSH_DRIVER_DATA)(int);
	typedef int ( *ADL2_ADAPTER_ACTIVE_GET ) (ADL_CONTEXT_HANDLE, int, int* );
	typedef int ( *ADL_ADAPTER_NUMBEROFADAPTERS_GET ) ( int* );
	typedef int ( *ADL_ADAPTER_ADAPTERINFO_GET ) ( LPAdapterInfo, int );
	typedef int ( *ADL_ADAPTERX2_CAPS) (int, int*);
	typedef int ( *ADL2_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);
	typedef int ( *ADL2_OVERDRIVEN_CAPABILITIES_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNCapabilities*);
	typedef int ( *ADL2_OVERDRIVEN_CAPABILITIESX2_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNCapabilities*);
	typedef int ( *ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
	typedef int ( *ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevelsX2*);
	typedef int ( *ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
	typedef int ( *ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevelsX2*);
	typedef int ( *ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
	typedef int ( *ADL2_OVERDRIVEN_MEMORYCLOCKSX2_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevelsX2*);
	typedef int ( *ADL2_OVERDRIVEN_MEMORYCLOCKS_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
	typedef int ( *ADL2_OVERDRIVEN_MEMORYCLOCKSX2_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevelsX2*);
	typedef int ( *ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceStatus*);
	typedef int ( *ADL2_OVERDRIVEN_FANCONTROL_GET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
	typedef int ( *ADL2_OVERDRIVEN_FANCONTROL_SET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
	typedef int ( *ADL2_OVERDRIVEN_POWERLIMIT_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
	typedef int ( *ADL2_OVERDRIVEN_POWERLIMIT_SET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
	typedef int ( *ADL2_OVERDRIVEN_TEMPERATURE_GET) (ADL_CONTEXT_HANDLE, int, int, int*);

	ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create;
	ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy;
	ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get;
	ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get;
	ADL_ADAPTERX2_CAPS ADL_AdapterX2_Caps;
	ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get;
	ADL2_OVERDRIVEN_CAPABILITIES_GET ADL2_OverdriveN_Capabilities_Get;
	ADL2_OVERDRIVEN_CAPABILITIESX2_GET ADL2_OverdriveN_CapabilitiesX2_Get;
	ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET ADL2_OverdriveN_SystemClocks_Get;
	ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_GET ADL2_OverdriveN_SystemClocksX2_Get;
	ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET ADL2_OverdriveN_SystemClocks_Set;
	ADL2_OVERDRIVEN_SYSTEMCLOCKSX2_SET ADL2_OverdriveN_SystemClocksX2_Set;
	ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET ADL2_OverdriveN_PerformanceStatus_Get;
	ADL2_OVERDRIVEN_FANCONTROL_GET ADL2_OverdriveN_FanControl_Get;
	ADL2_OVERDRIVEN_FANCONTROL_SET ADL2_OverdriveN_FanControl_Set;
	ADL2_OVERDRIVEN_POWERLIMIT_GET ADL2_OverdriveN_PowerLimit_Get;
	ADL2_OVERDRIVEN_POWERLIMIT_SET ADL2_OverdriveN_PowerLimit_Set;
	ADL2_OVERDRIVEN_MEMORYCLOCKS_GET ADL2_OverdriveN_MemoryClocks_Get;
	ADL2_OVERDRIVEN_MEMORYCLOCKSX2_GET ADL2_OverdriveN_MemoryClocksX2_Get;
	ADL2_OVERDRIVEN_MEMORYCLOCKS_GET ADL2_OverdriveN_MemoryClocks_Set;
	ADL2_OVERDRIVEN_MEMORYCLOCKSX2_GET ADL2_OverdriveN_MemoryClocksX2_Set;
	ADL2_OVERDRIVE_CAPS ADL2_Overdrive_Caps;
	ADL2_OVERDRIVEN_TEMPERATURE_GET ADL2_OverdriveN_Temperature_Get;

	std::map<int, int> adapters;

	ADL_CONTEXT_HANDLE context;
	LPAdapterInfo lpAdapterInfo;

#ifdef _WIN32
	HINSTANCE hDLL;
#else
	void* hDLL;
#endif // _WIN32

	std::map<int, ADLODNPerformanceLevelsX2*> systemClocks;
	std::map<int, ADLODNPerformanceLevelsX2*> memoryClocks;
	std::map<int, ADLODNFanControl*> fanControl;
	std::map<int, ADLODNPerformanceStatus*> performanceStatus;
	std::map<int, ADLODNPowerLimitSetting*> powerLimit;
	std::map<int, int> temperature;

	ADLODNPerformanceLevelsX2* OverdriveN_SystemClocksX2_Get(int adapterIndex);
	ADLODNPerformanceLevelsX2* OverdriveN_MemoryClocksX2_Get(int adapterIndex);
	ADLODNFanControl* OverdriveN_FanControl_Get(int adapterIndex);
	ADLODNPerformanceStatus* OverdriveN_PerformanceStatus_Get(int adapterIndex);
	ADLODNPowerLimitSetting* OverdriveN_PowerLimit_Get(int adapterIndex);
	int OverdriveN_Temperature_Get(int adapterIndex);

	void setSystemClock(int adapterIndex, int level, int clock, int vddc);
	void setMemoryClock(int adapterIndex, int level, int clock, int vddc);
	void setPowerLimit(int adapterIndex, int powerLimit);
	void setTargetTemperature(int adapterIndex, int targetTemperature);
	void setMinFanLimit(int adapterIndex, int minFanLimit);
};

#endif /* __ADL_H__ */