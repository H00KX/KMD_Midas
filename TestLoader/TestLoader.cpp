#include "Windows.h"
#include <iostream>

#define IO_CLEAR_UNLOADED_DRIVERS	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x707, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

typedef struct _CLEARUNLOADED
{
	NTSTATUS success;
} CLEARUNLOADED, *PCLEARUNLOADED;

#define DRIVER_NAME "midas"

std::string get_last_error_as_string()
{
	//Get the error message, if any.
	const auto error_message_id = GetLastError();
	if (error_message_id == 0)
		return std::string(); //No error message has been recorded

	LPSTR message_buffer = nullptr;
	const size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, error_message_id, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&message_buffer), 0, nullptr);

	std::string message(message_buffer, size);

	//Free the buffer.
	LocalFree(message_buffer);

	return message;
}

void load_driver()
{
	try
	{
		SERVICE_STATUS ss;

		const auto h_sc_manager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);

		if (h_sc_manager)
		{
			auto h_service = CreateServiceA(h_sc_manager, DRIVER_NAME, nullptr, SERVICE_START | DELETE | SERVICE_STOP, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, "C:\\Users\\User\\Desktop\\Midas.sys", nullptr, nullptr, nullptr, nullptr, nullptr);

			if (!h_service)
			{
				h_service = OpenServiceA(h_sc_manager, DRIVER_NAME, SERVICE_START | DELETE | SERVICE_STOP);
			}

			if (h_service)
			{
				if (!StartServiceW(h_service, 0, nullptr))
				{
					std::cout << get_last_error_as_string().c_str() << std::endl;
					std::cout << ("Driver may already be loaded...") << std::endl;
					ControlService(h_service, SERVICE_CONTROL_STOP, &ss);

					return;
				}
			}
			else
			{
				std::cout << ("Test") << std::endl;
			}

			CloseServiceHandle(h_sc_manager);
		}
		else
		{
#ifndef NDEBUG
			std::cout << h_sc_manager << std::endl;
#endif
			std::cout << ("Driver load error...Driver may already be loaded or you may not be running as admin...") << std::endl;
		}

	}
	catch (...)
	{
		std::cout << ("Error loading driver...") << std::endl;
	}

	std::cout << ("You may now launch the game...") << std::endl;
}

void unload_driver() {
	const auto h_sc_manager = OpenSCManagerA(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);

	if (h_sc_manager) {
		const auto h_service = OpenServiceA(h_sc_manager, DRIVER_NAME, SERVICE_START | DELETE | SERVICE_STOP);
		if (h_service) {
			SERVICE_STATUS ss;
			ControlService(h_service, SERVICE_CONTROL_STOP, &ss);
			DeleteService(h_service);
		}
		else std::cout << "Driver not loaded..." << std::endl;

		CloseServiceHandle(h_sc_manager);
	}
	else std::cout << "OpenSCManagerA failed..." << std::endl;


	std::cout << ("You may now launch the game...") << std::endl;
}

int main()
{
	SetLastError(0);

	//std::cout << "loading driver" << std::endl;
	//load_driver();
	//system("pause");

	std::cout << "connecting to driver" << std::endl;
	const auto h_driver = CreateFileA("\\\\.\\midas", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (!h_driver)
		std::cout << get_last_error_as_string().c_str() << std::endl;

	std::cout << h_driver << std::endl;

	system("pause");

	SetLastError(0);
	DWORD bytes;
	std::cout << DeviceIoControl(h_driver, IO_CLEAR_UNLOADED_DRIVERS, nullptr, 0, nullptr, 0, &bytes, nullptr) << std::endl;
	std::cout << get_last_error_as_string().c_str() << std::endl;
	std::cout << bytes << std::endl;

	system("pause");

	//unload_driver();

	return 0;
}

