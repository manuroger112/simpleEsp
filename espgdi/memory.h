#pragma once
#include <Windows.h>
#include <TlHelp32.h>


namespace VARS {
	

	DWORD GetProcess(const wchar_t* procName) {

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		
		PROCESSENTRY32W processEntry;
		processEntry.dwSize = sizeof(PROCESSENTRY32W);
		

		if (Process32FirstW(snapshot, &processEntry)) {
			
			

			do {
				if (_wcsicmp(processEntry.szExeFile, procName) == 0) {

					CloseHandle(snapshot);
					return processEntry.th32ProcessID;
					
					break;
				}


			} while (Process32NextW(snapshot, &processEntry));


		}
		else {
			return 1;
		}

		

	}




	uintptr_t GetModuleBaseAddress(DWORD processId, const wchar_t* moduleName) {
		HANDLE snapHandle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);

		MODULEENTRY32W modEntry;
		modEntry.dwSize = sizeof(MODULEENTRY32W);

		if (Module32FirstW(snapHandle, &modEntry)) {
			

			do {

				if (_wcsicmp(modEntry.szModule, moduleName) == 0) {
					CloseHandle(snapHandle);
					return (uintptr_t)modEntry.modBaseAddr;
					break;
				}


			} while (Module32NextW(snapHandle, &modEntry));



		}
		else {
			return 1;
		}


		
	}
	DWORD procId = GetProcess(L"csgo.exe");
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, false, procId);
	
	uintptr_t baseAddress = GetModuleBaseAddress(procId, L"client.dll");
	
	
	








	template<typename T>


	T memRead(uintptr_t locationAddress) {
		T value = { };
		ReadProcessMemory(processHandle, (LPVOID)locationAddress, &value, sizeof(T), NULL);
		return value;
	}


	template<typename T>


	T memWrite(uintptr_t locationAddress, T valueToWrite) {
		T value = { };
		return WriteProcessMemory(processHandle, (LPVOID)locationAddress, &valueToWrite, sizeof(T), NULL);
		
	}
	















	














}



