#include <iostream>
#include "Windows.h"
#include <string>
#include <thread>
#include <libloaderapi.h>

using namespace std;

void get_proc_id(const char* window_title, DWORD& process_id)
{
	GetWindowThreadProcessId(FindWindow(NULL, window_title), &process_id);
}

void error(const char* error_title, const char* error_msg)
{
	MessageBox(NULL, error_msg, error_title, NULL);
	exit(-1);
}

bool file_exists(string file_name)
{
	struct stat buffer;
	return(stat(file_name.c_str(), &buffer) == 0); // return true if file exist
}

int main()
{
	DWORD proc_id = NULL;
	char dll_path[MAX_PATH];
	const char* dll_name = "csgo_basic.dll";
	const char* window_title = "Counter-Strike: Global Offensive";

	if (!file_exists(dll_name))
	{
		error("file_exists", "file does not exist");
	}

	if (!GetFullPathName(dll_name, MAX_PATH, dll_path, nullptr))
	{
		error("GetFullPathName", "Failed to get full path");
	}

	get_proc_id(window_title, proc_id);
	if (proc_id == NULL)
	{
		error("get_proc_id", "Failed to get process ID");
	}

	HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, NULL, proc_id);
	if (!h_process)
	{
		error("OpenProcess", "Failed to open a handle to process");
	}

	void* allocated_memory = VirtualAllocEx(h_process, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!allocated_memory)
	{
		error("VirtualAllocEx", "Failed to allocate memory");
	}

	if (!WriteProcessMemory(h_process, allocated_memory, dll_path, MAX_PATH, nullptr))
	{
		error("WriteProcessMemory", "Failed to write process memory");
	}

	HANDLE h_thread = CreateRemoteThread(h_process, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocated_memory, NULL, nullptr);
	if (!h_thread)
	{
		error("CreateRemoteThread", "Failed to create remote thread");
	}

	CloseHandle(h_process);
	VirtualFreeEx(h_process, allocated_memory, NULL, MEM_RELEASE);
	MessageBox(0, "Injected successfully!", "Success", 0);

}