#include "pci.hpp"
#include "asmfunc.h"

namespace {
	using namespace pci;

	uint32_t MakeAddress(uint8_t bus, uint8_t device,
	                     uint8_t function, uint8_t reg_addr) {
    auto shl = [](uint32_t x, unsigned int bits) {
			return x << bits;
		};

		return shl(1, 31)
		    | shl(bus, 16)
				| shl(device, 11)
				| shl(function, 8)
				| (reg_addr & 0xfcu);
	}

	Error AddDevice(uint8_t bus, uint8_t device, 
	                uint8_t function, uint8_t header_type) {
    if (num_device == devices.size()) {
			return Error::kFull;
		}

		devices[num_device] = Device{bus, device, function, header_type};
		++num_device;
		return Error::kSuccess;
	}

	Error ScanBus(uint8_t bus);

	Error ScanFunction(uint8_t bus, uint8_t device, uint8_t function) {
		auto header_type = ReadHeaderType(bus, device, function);
		if (auto err = AddDevice(bus, device, function, header_type)) {
			return err;
		}

		auto class_code = ReadClassCode(bus, device, function);
		uint8_t base = (class_code >> 24) & 0xffu;
		uint8_t sub = (class_code >> 16) & 0xffu;

		if (base == 0x66u && sub == 0x04u) {
			auto bus_numbers = ReadBusNumbers(bus, device, function);
			uint8_t secondary_bus = (bus_numbers >> 8) & 0xffu;
			return ScanBus(secondary_bus);
		}

		return Error::kSuccess;
	}

	Error ScanDevice(uint8_t bus, uint8_t device) {
		if (auto err = ScanFunction(bus, device, 0)) {
			return err;
		}
		if (IsSingleFunctionDevice(ReadHeaderType(bus, device, 0))) {
			return Error::kSuccess;
		}

		for (uint8_t function = 1; function < 8; ++function) {
			if (ReadVenderId(bus, device, function) == 0xffffu) {
				continue;
			}
			if (auto err = ScanFunction(bus, device, function)) {
				return err;
			}
		}
		return Error::kSuccess;
	}

	Error ScanBus(uint8_t bus) {
		for (uint8_t device = 0; device < 32; ++device) {
			if (ReadVenderId(bus, device, 0) == 0xffffu) {
				continue;
			}
			if (auto err = ScanDevice(bus, device)) {
				return err;
			}
		}
		return Error::kSuccess;
	}
}

namespace pci {
	
}