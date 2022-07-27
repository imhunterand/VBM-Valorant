#include "pattern.hpp"
#include "util.hpp"
#include "raid_extension.hpp"
#include <classpnp.h>
#include <ntifs.h>s

NTSTATUS ProcessReadWriteMemory(PEPROCESS SourceProcess, PVOID SourceAddress, PEPROCESS TargetProcess, PVOID TargetAddress, SIZE_T Size)
{
	SIZE_T Bytes = 0;

	if (NT_SUCCESS(MmCopyVirtualMemory(SourceProcess, SourceAddress, TargetProcess, TargetAddress, Size, UserMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

NTSTATUS driver_start( )
{
	std::unique_ptr< DRIVER_OBJECT, decltype( &ObfDereferenceObject ) > disk_object( nullptr, &ObfDereferenceObject );
	
	UNICODE_STRING driver_unicode{};
	RtlInitUnicodeString( &driver_unicode, L"\\Driver\\Disk" );
	
	ObReferenceObjectByName( &driver_unicode, OBJ_CASE_INSENSITIVE, nullptr, 0, *IoDriverObjectType, KernelMode, nullptr, reinterpret_cast< void** >( disk_object.get( ) ) );

	if ( !disk_object.get( ) )
		return STATUS_UNSUCCESSFUL;

	memory::initialize( L"disk.sys" );
	const auto DiskEnableDisableFailurePrediction = reinterpret_cast< NTSTATUS( __fastcall* )( PFUNCTIONAL_DEVICE_EXTENSION, BOOLEAN ) >( memory::from_pattern( "\x48\x89\x5c\x24\x00\x48\x89\x74\x24\x00\x57\x48\x81\xec\x00\x00\x00\x00\x48\x8b\x05\x00\x00\x00\x00\x48\x33\xc4\x48\x89\x84\x24\x00\x00\x00\x00\x48\x8b\x59\x60\x48\x8b\xf1\x40\x8a\xfa\x8b\x4b\x10", "xxxx?xxxx?xxxx????xxx????xxxxxxx????xxxxxxxxxxxxx" ) );

	if ( !DiskEnableDisableFailurePrediction )
		return STATUS_UNSUCCESSFUL;

	memory::initialize( L"storport.sys" );
	const auto RaidUnitRegisterInterfaces_address = memory::from_pattern( "\xe8\x00\x00\x00\x00\x48\x8b\xcb\xe8\x00\x00\x00\x00\x85\xc0\x74\x0a", "x????xxxx????xxxx" );

	if ( !RaidUnitRegisterInterfaces_address )
		return STATUS_UNSUCCESSFUL;

	const auto RaidUnitRegisterInterfaces = reinterpret_cast< NTSTATUS( __fastcall* )( RAID_UNIT_EXTENSION* ) >( RaidUnitRegisterInterfaces_address + 5 + *reinterpret_cast< std::int32_t* >( RaidUnitRegisterInterfaces_address + 1 ) );

	const auto seed = __rdtsc( );

	for (auto current_object = disk_object->DeviceObject; current_object != nullptr; current_object = current_object->NextDevice )
	{
		const auto fd_extension = static_cast< PFUNCTIONAL_DEVICE_EXTENSION >( current_object->DeviceExtension );

		if ( !fd_extension )
			continue;

		const auto fs_device = IoGetDeviceAttachmentBaseRef( current_object );

		if ( !fs_device || fs_device->DeviceType != FILE_DEVICE_DISK || !fs_device->DeviceExtension )
			continue;

		const auto raid_extension = static_cast< PRAID_UNIT_EXTENSION >( fs_device->DeviceExtension );
		const auto identity = reinterpret_cast< PSTOR_SCSI_IDENTITY >( std::uintptr_t( raid_extension ) + 0x68 ); // this offset changes per windows build, you figure it out
		const auto fdo_descriptor = fd_extension->DeviceDescriptor;

		if ( !fdo_descriptor )
		{
			ObfDereferenceObject( fs_device );
			continue;
		}
		
		const auto fdo_serial = reinterpret_cast< char* >( fdo_descriptor ) + fdo_descriptor->SerialNumberOffset;

		serializer::randomize( seed, fdo_serial );

		identity->SerialNumber.Length = static_cast< USHORT >( std::strlen( fdo_serial ) );
		memset( identity->SerialNumber.Buffer, 0, identity->SerialNumber.Length );
		memcpy( identity->SerialNumber.Buffer, fdo_serial, identity->SerialNumber.Length );

		DiskEnableDisableFailurePrediction( fd_extension, FALSE );
		RaidUnitRegisterInterfaces( raid_extension );
		
		const auto raid_extension = static_cast< PRAID_UNIT_EXTENSION >( fs_device->DeviceExtension );
		const auto identity = reinterpret_cast< PSTOR_SCSI_IDENTITY >( std::uintptr_t( raid_extension ) + 0x68 ); // this offset changes per windows build, you figure it out
		const auto fdo_descriptor = fd_extension->DeviceDescriptor;
	}
		
	return STATUS_SUCCESS;
}


bool Client::setupEncryption() {
	std::string SetupKey = sendrecieve(ENC_KEY + _xor_(";") + IV);
	std::string serverCipher = SetupKey.substr(0, SetupKey.find(_xor_(";"))).data();
	std::string serverIv = SetupKey.substr(SetupKey.find(_xor_(";")) + 1).data();

	if (ENC_KEY == serverCipher) {
		if (IV == serverIv) {
			// Client-Server authentication success
			std::string ServerVersion = sendrecieve(VERSION);
			// Check version control:
			if (VERSION == ServerVersion) {
				std::string successGetUsername = _xor_("SUCCESS_GET_USERNAME");
				std::string getUsername = sendrecieve(stringer(_xor_("SPOOFER_GET_USERNAME;"), getHWinfo64()));
				if (getUsername.size() == successGetUsername.size()) return true;
				else ERRORLOG(_xor_("ERROR 3131"));
			}
			else ERRORLOG(_xor_("ERROR 103"));
		}
		else ERRORLOG(_xor_("ERROR 102"));
	}
	else ERRORLOG(_xor_("ERROR 101"));
}
