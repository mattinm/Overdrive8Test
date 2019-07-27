// Overdrive8Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "adl_sdk.h"
#include "adl_structures.h"

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

using namespace std;

// function pointers
typedef int(*ADL_MAIN_CONTROL_CREATE)				(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)				();
typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET)		(int*);
typedef int(*ADL_ADAPTER_ADAPTERINFO_GET)			(LPAdapterInfo, int);
typedef int(*ADL2_OVERDRIVE_CAPS)					(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* iSupported, int* iEnabled, int* iVersion);

typedef int(*ADL2_OVERDRIVE8_INIT_SETTING_GET)		(ADL_CONTEXT_HANDLE, int, ADLOD8InitSetting*);
typedef int(*ADL2_OVERDRIVE8_CURRENT_SETTING_GET)	(ADL_CONTEXT_HANDLE, int, ADLOD8CurrentSetting*);
typedef int(*ADL2_OVERDRIVE8_SETTING_SET)			(ADL_CONTEXT_HANDLE, int, ADLOD8SetSetting*, ADLOD8CurrentSetting*);
typedef int(*ADL2_NEW_QUERYPMLOGDATA_GET)			(ADL_CONTEXT_HANDLE, int, ADLPMLogDataOutput*);

typedef int(*ADL2_OVERDRIVE8_INIT_SETTINGX2_GET)	(ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpOverdrive8Capabilities, int* lpNumberOfFeatures, ADLOD8SingleInitSetting** lppInitSettingList);
typedef int(*ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int* lpNumberOfFeatures, int** lppCurrentSettingList);


// globals
HINSTANCE hDll = nullptr;
ADL_CONTEXT_HANDLE context = nullptr;
int numberOfAdapters = 0;
LPAdapterInfo adapterInfo = nullptr;

// functions from the DLL
ADL_MAIN_CONTROL_CREATE					ADL_Main_Control_Create					= nullptr;
ADL_MAIN_CONTROL_DESTROY				ADL_Main_Control_Destroy				= nullptr;
ADL_ADAPTER_NUMBEROFADAPTERS_GET		ADL_Adapter_NumberOfAdapters_Get		= nullptr;
ADL_ADAPTER_ADAPTERINFO_GET				ADL_Adapter_AdapterInfo_Get				= nullptr;
ADL2_OVERDRIVE_CAPS						ADL2_Overdrive_Caps						= nullptr;

ADL2_OVERDRIVE8_INIT_SETTING_GET		ADL2_Overdrive8_Init_Setting_Get		= nullptr;
ADL2_OVERDRIVE8_CURRENT_SETTING_GET		ADL2_Overdrive8_Current_Setting_Get		= nullptr;
ADL2_OVERDRIVE8_SETTING_SET				ADL2_Overdrive8_Setting_Set				= nullptr;
ADL2_NEW_QUERYPMLOGDATA_GET				ADL2_New_QueryPMLogData_Get				= nullptr;

ADL2_OVERDRIVE8_INIT_SETTINGX2_GET		ADL2_Overdrive8_Init_SettingX2_Get		= nullptr;
ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET	ADL2_Overdrive8_Current_SettingX2_Get	= nullptr;

void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
	void* lpBuffer = malloc(iSize);
	return lpBuffer;
}

int error(int number, string msg)
{
	cout << "[ERROR]" << msg << endl;
	return number;
}

int initialize_adl()
{
	if (hDll != nullptr)
		return error(0, "ADL library already loaded.");

	hDll = LoadLibrary(TEXT("atiadlxx.dll"));
	if (hDll == nullptr)
		hDll = LoadLibrary(TEXT("atiadlxy.dll"));

	if (hDll == nullptr)
		return error(0, "ADL library failed to load.");

	// setup our function pointers
	ADL_Main_Control_Create				= (ADL_MAIN_CONTROL_CREATE)					GetProcAddress(hDll, "ADL_Main_Control_Create");
	ADL_Main_Control_Destroy			= (ADL_MAIN_CONTROL_DESTROY)				GetProcAddress(hDll, "ADL_Main_Control_Destroy");
	ADL_Adapter_NumberOfAdapters_Get	= (ADL_ADAPTER_NUMBEROFADAPTERS_GET)		GetProcAddress(hDll, "ADL_Adapter_NumberOfAdapters_Get");
	ADL_Adapter_AdapterInfo_Get			= (ADL_ADAPTER_ADAPTERINFO_GET)				GetProcAddress(hDll, "ADL_Adapter_AdapterInfo_Get");
	ADL2_Overdrive_Caps					= (ADL2_OVERDRIVE_CAPS)						GetProcAddress(hDll, "ADL2_Overdrive_Caps");

	ADL2_Overdrive8_Init_Setting_Get = (ADL2_OVERDRIVE8_INIT_SETTING_GET)			GetProcAddress(hDll, "ADL2_Overdrive8_Init_Setting_Get");
	ADL2_Overdrive8_Current_Setting_Get = (ADL2_OVERDRIVE8_CURRENT_SETTING_GET)		GetProcAddress(hDll, "ADL2_Overdrive8_Current_Setting_Get");
	ADL2_Overdrive8_Setting_Set = (ADL2_OVERDRIVE8_SETTING_SET)						GetProcAddress(hDll, "ADL2_Overdrive8_Setting_Set");
	ADL2_New_QueryPMLogData_Get = (ADL2_NEW_QUERYPMLOGDATA_GET)						GetProcAddress(hDll, "ADL2_New_QueryPMLogData_Get");

	ADL2_Overdrive8_Init_SettingX2_Get = (ADL2_OVERDRIVE8_INIT_SETTINGX2_GET)		GetProcAddress(hDll, "ADL2_Overdrive8_Init_SettingX2_Get");
	ADL2_Overdrive8_Current_SettingX2_Get = (ADL2_OVERDRIVE8_CURRENT_SETTINGX2_GET)	GetProcAddress(hDll, "ADL2_Overdrive8_Current_SettingX2_Get");

	if (
		nullptr == ADL_Main_Control_Create				||
		nullptr == ADL_Main_Control_Destroy				||
		nullptr == ADL_Adapter_NumberOfAdapters_Get		||
		nullptr == ADL_Adapter_AdapterInfo_Get			||
		nullptr == ADL2_Overdrive_Caps					||
		nullptr == ADL2_Overdrive8_Init_Setting_Get		||
		nullptr == ADL2_Overdrive8_Current_Setting_Get	||
		nullptr == ADL2_Overdrive8_Setting_Set			||
		nullptr == ADL2_New_QueryPMLogData_Get			||
		nullptr == ADL2_Overdrive8_Init_SettingX2_Get	||
		nullptr == ADL2_Overdrive8_Current_SettingX2_Get
	)
		return error(0, "Failed to get ADL function pointers.");

	if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
		return error(ADL_ERR, "Failed to initialize nested ADL2 context.");

	return 1;
}

void cleanup_adl()
{
	if (ADL_Main_Control_Destroy != nullptr)
		ADL_Main_Control_Destroy();

	if (adapterInfo != nullptr)
	{
		free(adapterInfo);
		adapterInfo = nullptr;
	}

	if (hDll != nullptr)
	{
		FreeLibrary(hDll);
		hDll = nullptr;
	}
}

int GetOD8InitSetting(int iAdapterIndex, ADLOD8InitSetting& odInitSetting)
{
	int ret = ADL_ERR;

	memset(&odInitSetting, 0, sizeof(odInitSetting));
	odInitSetting.count = OD8_COUNT;

	int overdrive8Capabilities;
	int numberOfFeatures = OD8_COUNT;
	ADLOD8SingleInitSetting* lpInitSettingList = nullptr;

	if (nullptr != ADL2_Overdrive8_Init_SettingX2_Get)
	{
		ret = ADL2_Overdrive8_Init_SettingX2_Get(context, iAdapterIndex, &overdrive8Capabilities, &numberOfFeatures, &lpInitSettingList);
		if (ADL_OK == ret)
		{
			odInitSetting.count = numberOfFeatures > OD8_COUNT ? OD8_COUNT : numberOfFeatures;
			odInitSetting.overdrive8Capabilities = overdrive8Capabilities;
			for (int i = 0; i < odInitSetting.count; i++)
			{
				odInitSetting.od8SettingTable[i].defaultValue	= lpInitSettingList[i].defaultValue;
				odInitSetting.od8SettingTable[i].featureID		= lpInitSettingList[i].featureID;
				odInitSetting.od8SettingTable[i].maxValue		= lpInitSettingList[i].maxValue;
				odInitSetting.od8SettingTable[i].minValue		= lpInitSettingList[i].minValue;
			}
		}
		else
			cout << "[ERROR] ADL2_Overdrive8_Init_SettingX2_Get is failed." << endl;

		free(lpInitSettingList);
		return ret;
	}

	if (nullptr != ADL2_Overdrive8_Init_Setting_Get)
	{
		ret = ADL2_Overdrive8_Init_Setting_Get(context, iAdapterIndex, &odInitSetting);
		if (ADL_OK == ret)
		{
			// do stuff?
		}
		else
			cout << "[ERROR] ADL2_Overdrive8_Init_Setting_Get is failed." << endl;

		return ret;
	}

	cout << "[ERROR] No Init_Setting_Get function defined." << endl;
	return ADL_ERR;
}

int GetOD8CurrentSetting(int iAdapterIndex, ADLOD8CurrentSetting& odCurrentSetting)
{
	int ret = ADL_ERR;

	memset(&odCurrentSetting, 0, sizeof(ADLOD8CurrentSetting));
	odCurrentSetting.count = OD8_COUNT;

	int numberOfFeaturesCurrent = OD8_COUNT;
	int* lpCurrentSettingList = NULL;

	if (nullptr != ADL2_Overdrive8_Current_SettingX2_Get)
	{
		ret = ADL2_Overdrive8_Current_SettingX2_Get(context, iAdapterIndex, &numberOfFeaturesCurrent, &lpCurrentSettingList);
		if (ADL_OK == ret)
		{
			odCurrentSetting.count = numberOfFeaturesCurrent > OD8_COUNT ? OD8_COUNT : numberOfFeaturesCurrent;
			for (int i = 0; i < odCurrentSetting.count; i++)
				odCurrentSetting.Od8SettingTable[i] = lpCurrentSettingList[i];
		}
		else
			cout << "[ERROR] ADL2_Overdrive8_Current_SettingX2_Get is failed." << endl;

		free(lpCurrentSettingList);
		return ret;
	}

	if (nullptr != ADL2_Overdrive8_Current_Setting_Get)
	{
		ret = ADL2_Overdrive8_Current_Setting_Get(context, iAdapterIndex, &odCurrentSetting);
		if (ADL_OK == ret)
		{
			// do stuff
		}
		else
			cout << "[ERROR] ADL2_Overdrive8_Current_Setting_Get is failed." << endl;

		return ret;
	}

	cout << "[ERROR] No Current_Setting_Get function defined." << endl;
	return ADL_ERR;
}

/**
 * Get the current temperature. Print out the EDGE and HOTSPOT temps, but only return the EDGE temp.
 */
int get_temperature()
{
	AdapterInfo *info = nullptr;
	int supported = 0, enabled = 0, version = 0;

	info = &adapterInfo[0];

	if (info->iBusNumber < 0)
	{
		cout << "[ERROR] Unknown bus number: " << info->iBusNumber << endl;
		return ADL_ERR;
	}

	if (ADL_OK != ADL2_Overdrive_Caps(context, info->iAdapterIndex, &supported, &enabled, &version))
	{
		cout << "[ERROR] Unable to get caps." << endl;
		return ADL_ERR;
	}

	if (version != 8)
	{
		cout << "[ERROR] Incorrect version (expected 8): " << version << endl;
		return ADL_ERR;
	}

	ADLOD8InitSetting	odInitSetting;
	if (ADL_OK != GetOD8InitSetting(info->iAdapterIndex, odInitSetting))
	{
		cout << "[ERROR] GetInitSetting failed." << endl;
		return ADL_ERR;
	}

	ADLOD8CurrentSetting odCurrentSetting;
	if (ADL_OK != GetOD8CurrentSetting(info->iAdapterIndex, odCurrentSetting))
	{
		cout << "[ERROR] GetCurrentSetting failed." << endl;
		return ADL_ERR;
	}

	ADLPMLogDataOutput odlpDataOutput;
	memset(&odlpDataOutput, 0, sizeof(odlpDataOutput));

	if (ADL_OK != ADL2_New_QueryPMLogData_Get(context, info->iAdapterIndex, &odlpDataOutput))
	{
		cout << "[ERROR] ADL2_New_QueryPMLogData_Get failed." << endl;
		return ADL_ERR;
	}

	if	(
			((odInitSetting.overdrive8Capabilities & ADL_OD8_TEMPERATURE_SYSTEM) == ADL_OD8_TEMPERATURE_SYSTEM) ||
			((odInitSetting.overdrive8Capabilities & ADL_OD8_TEMPERATURE_FAN) == ADL_OD8_TEMPERATURE_FAN) ||
			((odInitSetting.overdrive8Capabilities & ADL_OD8_POWER_LIMIT) == ADL_OD8_POWER_LIMIT)
		)
	{
		cout << "ADLSensorType: PMLOG_TEMPERATURE_EDGE - Current Temp" << endl;
		cout << "PMLOG_TEMPERATURE_EDGE.supported: " << odlpDataOutput.sensors[PMLOG_TEMPERATURE_EDGE].supported << endl;
		cout << "PMLOG_TEMPERATURE_EDGE.value: " << odlpDataOutput.sensors[PMLOG_TEMPERATURE_EDGE].value << endl;

		cout << "ADLSensorType: PMLOG_TEMPERATURE_HOTSPOT - Junction Temp" << endl;
		cout << "PMLOG_TEMPERATURE_HOTSPOT.supported: " << odlpDataOutput.sensors[PMLOG_TEMPERATURE_HOTSPOT].supported << endl;
		cout << "PMLOG_TEMPERATURE_HOTSPOT.value: " << odlpDataOutput.sensors[PMLOG_TEMPERATURE_HOTSPOT].value << endl;

		return odlpDataOutput.sensors[PMLOG_TEMPERATURE_EDGE].value;
	}

	return ADL_ERR;
}

/**
 * Get the current fan speed. One issue with this is that it's currently only reporting the value in RPMs.
 * The only way to really convert this back would be to determine the max RPM and assume a linear curve, but
 * that probably isn't fully accurate. Not sure how reporting for this will really work.
 */
int get_fan_settings()
{
	AdapterInfo* info = nullptr;
	int supported = 0, enabled = 0, version = 0;

	info = &adapterInfo[0];

	if (info->iBusNumber < 0)
	{
		cout << "[ERROR] Unknown bus number: " << info->iBusNumber << endl;
		return ADL_ERR;
	}

	if (ADL_OK != ADL2_Overdrive_Caps(context, info->iAdapterIndex, &supported, &enabled, &version))
	{
		cout << "[ERROR] Unable to get caps." << endl;
		return ADL_ERR;
	}

	if (version != 8)
	{
		cout << "[ERROR] Incorrect version (expected 8): " << version << endl;
		return ADL_ERR;
	}

	ADLOD8InitSetting	odInitSetting;
	if (ADL_OK != GetOD8InitSetting(info->iAdapterIndex, odInitSetting))
	{
		cout << "[ERROR] GetInitSetting failed." << endl;
		return ADL_ERR;
	}

	ADLOD8CurrentSetting odCurrentSetting;
	if (ADL_OK != GetOD8CurrentSetting(info->iAdapterIndex, odCurrentSetting))
	{
		cout << "[ERROR] GetCurrentSetting failed." << endl;
		return ADL_ERR;
	}

	ADLPMLogDataOutput odlpDataOutput;
	memset(&odlpDataOutput, 0, sizeof(odlpDataOutput));

	if (ADL_OK != ADL2_New_QueryPMLogData_Get(context, info->iAdapterIndex, &odlpDataOutput))
	{
		cout << "[ERROR] ADL2_New_QueryPMLogData_Get failed." << endl;
		return ADL_ERR;
	}

	if (
			(odInitSetting.overdrive8Capabilities & ADL_OD8_ACOUSTIC_LIMIT_SCLK) == ADL_OD8_ACOUSTIC_LIMIT_SCLK ||
			(odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_SPEED_MIN) == ADL_OD8_FAN_SPEED_MIN
		)
	{
		//odInitSetting.od8SettingTable
		cout << "ADLSensorType: PMLOG_FAN_RPM" << endl;
		cout << "PMLOG_FAN_RPM.supported: " << odlpDataOutput.sensors[PMLOG_FAN_RPM].supported << endl;
		cout << "PMLOG_FAN_RPM.value: " << odlpDataOutput.sensors[PMLOG_FAN_RPM].value << endl;

		return odlpDataOutput.sensors[PMLOG_FAN_RPM].value;
	}
	else if ((odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_CURVE) == ADL_OD8_FAN_CURVE)
	{
		cout << "ADLSensorType: PMLOG_FAN_RPM" << endl;
		cout << "PMLOG_FAN_RPM.value: " << odlpDataOutput.sensors[PMLOG_FAN_RPM].value << endl;
		return odlpDataOutput.sensors[PMLOG_FAN_RPM].value;
	}
	else
		cout << "OD8 Fan Setting failed." << endl;

	return ADL_ERR;
}

/**
 * Set the fan curve to a given PWM value by making the "curve" flat. There seems to still be a little ramping
 * done by the driver when the PWM is set to a lower value than current.
 */
int set_fan_curve(int pwm)
{
	AdapterInfo* info = nullptr;
	int supported = 0, enabled = 0, version = 0;

	info = &adapterInfo[0];

	if (info->iBusNumber < 0)
	{
		cout << "[ERROR] Unknown bus number: " << info->iBusNumber << endl;
		return ADL_ERR;
	}

	if (ADL_OK != ADL2_Overdrive_Caps(context, info->iAdapterIndex, &supported, &enabled, &version))
	{
		cout << "[ERROR] Unable to get caps." << endl;
		return ADL_ERR;
	}

	if (version != 8)
	{
		cout << "[ERROR] Incorrect version (expected 8): " << version << endl;
		return ADL_ERR;
	}

	ADLOD8InitSetting	odInitSetting;
	if (ADL_OK != GetOD8InitSetting(info->iAdapterIndex, odInitSetting))
	{
		cout << "[ERROR] GetInitSetting failed." << endl;
		return ADL_ERR;
	}

	ADLOD8CurrentSetting odCurrentSetting;
	if (ADL_OK != GetOD8CurrentSetting(info->iAdapterIndex, odCurrentSetting))
	{
		cout << "[ERROR] GetCurrentSetting failed." << endl;
		return ADL_ERR;
	}

	if (!(odInitSetting.overdrive8Capabilities & ADL_OD8_FAN_CURVE))
	{
		cout << "[ERROR] Fan curve not supported." << endl;
		return ADL_ERR;
	}

	ADLOD8SetSetting odSetSetting;
	memset(&odSetSetting, 0, sizeof(odSetSetting));
	odSetSetting.count = OD8_COUNT;

	// make sure we're within the fan curve range
	if (odInitSetting.od8SettingTable[OD8_FAN_CURVE_SPEED_5].minValue > pwm || odInitSetting.od8SettingTable[OD8_FAN_CURVE_SPEED_5].maxValue < pwm)
	{
		cout << "Fan range should be between " << odInitSetting.od8SettingTable[OD8_FAN_CURVE_SPEED_5].minValue << " and " << odInitSetting.od8SettingTable[OD8_FAN_CURVE_SPEED_5].maxValue << endl;
		return ADL_ERR;
	}

	// start at a temp value that make sense
	int temp = odInitSetting.od8SettingTable[OD8_FAN_CURVE_TEMPERATURE_5].minValue;
	for (int i = OD8_FAN_CURVE_TEMPERATURE_1; i <= OD8_FAN_CURVE_TEMPERATURE_5; i = i + 2)
	{
		odSetSetting.od8SettingTable[i].reset = false;
		odSetSetting.od8SettingTable[i].requested = 1;
		odSetSetting.od8SettingTable[i].value = temp++;
	}

	for (int i = OD8_FAN_CURVE_SPEED_1; i <= OD8_FAN_CURVE_SPEED_5; i = i + 2)
	{
		odSetSetting.od8SettingTable[i].reset = false;
		odSetSetting.od8SettingTable[i].requested = 1;
		odSetSetting.od8SettingTable[i].value = pwm;
	}

	if (ADL_OK == ADL2_Overdrive8_Setting_Set(context, info->iAdapterIndex, &odSetSetting, &odCurrentSetting))
	{
		cout << "ADL2_Overdrive8_Setting_Set is success" << endl;
		return ADL_OK;
	}
	else
		cout << "ADL2_Overdrive8_Setting_Set is failed" << endl;

	return ADL_ERR;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		cout << "Usage: ./<program> fan_speed" << endl;
		return 1;
	}

	int pwm = std::atoi(argv[1]);

	if (initialize_adl() <= 0)
	{
		cleanup_adl();
		return 1;
	}

	cout << "[LOG] Successfully created ADL2 context." << endl;

	if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&numberOfAdapters))
	{
		cleanup_adl();
		return error(1, "Cannot get the number of adapters.");
	}

	adapterInfo = (LPAdapterInfo)malloc(sizeof(*adapterInfo) * numberOfAdapters);
	if (adapterInfo == 0)
	{
		cleanup_adl();
		return error(1, "Out of memory.");
	}
	memset(adapterInfo, 0, sizeof(*adapterInfo) * numberOfAdapters);
	
	if (ADL_OK != ADL_Adapter_AdapterInfo_Get(adapterInfo, sizeof(*adapterInfo) * numberOfAdapters))
	{
		cleanup_adl();
		return error(1, "Cannot get the adapter information.");
	}

	int temp = get_temperature();
	cout << endl << endl << "TEMP FOUND: " << temp << endl << endl;

	int fan = get_fan_settings();
	cout << endl << endl << "FAN FOUND: " << fan << endl << endl;

	// set the fan curve based on the user input
	set_fan_curve(pwm);

	cleanup_adl();
	return 0;
}
