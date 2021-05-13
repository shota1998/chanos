#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include <Guid/FileInfo.h>

// UINTN ... Unsigned value of native width.
// gBS ... Global variable representing the boot service.
// gBS->GetMemoryMap ... Get the memory map at the time of the function call.
//  Write the Map to the memory specified by the MemoryMap argument.
//  If the function completes successfully, EFI_SUCCESS is returned.
//
//  -- Paramators --
//  1 in: Size of the memory  for memory map writing.
//  1 out: Actual memory size.
//  2: Pointer to the beginning of the memory for memory map writing.
//  3: Memory map identification ID. Changes each time it is updated.
//  4: Size of each memory discriptor. 
//  5: ?

struct MemoryMap {
  UINTN buffer_size;
  VOID* buffer;
  UINTN map_size;
  UINTN map_key;
  UINTN descriptor_size;
  UINT32 descriptor_version;
};

EFI_STATUS GetMemoryMap(struct MemoryMap* map) {
  if (map->buffer == NULL) {
    return EFI_BUFFER_TOO_SMALL;
  }

  map->map_size = map->buffer_size;
  return gBS->GetMemoryMap(
      &map->map_size,
      (EFI_MEMORY_DESCRIPTOR*)map->buffer,
      &map->map_key,
      &map->descriptor_size,
      &map->descriptor_version);
}

const CHAR16* GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
  switch (type) {
     case EfiReservedMemoryType: return L"EfiReservedMemoryType";
     case EfiLoaderCode: return L"EfiLoaderCode";
     case EfiLoaderData: return L"EfiLoaderData";
     case EfiBootServicesCode: return L"EfiBootServicesCode";
     case EfiBootServicesData: return L"EfiBootServicesData";
     case EfiRuntimeServicesCode: return L"EfiRuntimeServicesCode";
     case EfiRuntimeServicesData: return L"EfiRuntimeServicesData";
     case EfiConventionalMemory: return L"EfiConventionalMemory";
     case EfiUnusableMemory: return L"EfiUnusableMemory";
     case EfiACPIReclaimMemory: return L"EfiACPIReclaimMemory";
     case EfiACPIMemoryNVS: return L"EfiACPIMemoryNVS";
     case EfiMemoryMappedIO: return L"EfiMemoryMappedIO";
     case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
     case EfiPalCode: return L"EfiPalCode";
     case EfiPersistentMemory: return L"EfiPersistentMemory";
     case EfiMaxMemoryType: return L"EfiMaxMemoryType";
     default: return L"InvalidMemoryType";
   }
}

// AsciiStrLen() ... Returns the length of a Null-terminated ASCII string.
// EFI_PHYSICAL_ADDRESS ... 64-bit physical memory address.
// AsciiSPrint ... Like C's "sprintf".
// desc->Attribute ... Attributes of the memory region that describe the bit mask of capabilities for that memory region, and not necessarily the current settings for that memory region.
// lu ... unsigned long.

EFI_STATUS SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file){
  CHAR8 buf[256];
  UINTN len;

  CHAR8* header =
    "Index, Type, Type(name), PhisicalStart, NumberOsPages, Attribute\n";
  len = AsciiStrLen(header);
  file->Write(file, &len, header);

  Print(L"map->buffer = %081x, map->map_size = %081x\n",
      map->buffer, map->map_size);

  EFI_PHYSICAL_ADDRESS iter;
  int i;
  for ( iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i = 0;
        iter < (EFI_PHYSICAL_ADDRESS)map->buffer + map->map_size;
	      iter += map->descriptor_size, i++) {
          EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter;
          len = AsciiSPrint(
                  buf, sizeof(buf),
                	"%u, %x, %-ls, %081x, %lx, %lx\n",
                	i, desc->Type, GetMemoryTypeUnicode(desc->Type),
                	desc->PhysicalStart, desc->NumberOfPages,
                	desc->Attribute & 0xffffflu);
          file->Write(file, &len, buf);
   }

   return EFI_SUCCESS;
}

// EFI_LOADED_IMAGE_PROTOCOL ... Can be used on any image handle to obtain information about the loaded image.
// EFI_SIMPLE_FILE_SYSTEM_PROTOCOL ... the programmatic access to the FAT (12,16,32) file system specified in 
//    UEFI 2.0. 
// gBS->OpenProtocol ... https://edk2-docs.gitbook.io/edk-ii-uefi-driver-writer-s-guide/5_uefi_services/51_services_that_uefi_drivers_commonly_use/513_handle_database_and_protocol_services#example-38-openprotocol-function-prototype

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

  gBS->OpenProtocol(
      image_handle,
      &gEfiLoadedImageProtocolGuid,
      (VOID**)&loaded_image,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  gBS->OpenProtocol(
      loaded_image->DeviceHandle,
      &gEfiSimpleFileSystemProtocolGuid,
      (VOID**)&fs,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  fs->OpenVolume(fs, root);

  return EFI_SUCCESS;
}

// LocateHandleBuffer ... Retrieve the list of all the handles in the handle database. 
// @ByProtocol(searche option) ... Retrieve all handles in the handle database that support a specified protocol.
// @(Specify Protocol) ... .
// @(???) ...
// @HandleCount ... The number of handles in the handle database.
// @HandleBuffer  ... The array of handle values.
//
// OpenProtocol
EFI_STATUS OpenGOP(EFI_HANDLE image_handle,
                   EFI_GRAPHICS_OUTPUT_PROTOCOL** gop) {
  UINTN num_gop_handles = 0;
  EFI_HANDLE* gop_handles = NULL;
  gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiGraphicsOutputProtocolGuid,
      NULL,
      &num_gop_handles,
      &gop_handles);

  gBS->OpenProtocol(


// EFIAPI ...  
// EFI_HANDLE ... ImageHandle refers to the image handle of the UEFI application. 
// EFI_SYSTEM_TABLE ... SystemTable is the pointer to the EFI System Table.
//  L ... Encode wide characters with UCS2.
//
// EFI_FILE_PROTOCOL ... Provides file IO access to supported file systems.
// root_dir->Open ... Make "memmap" file at root dir.
// SaveMemoryMap ... Save memmap to " ~/memmap ".
// kernel_file->GetInfo() ... EFI_FILE_INFO will be written in file_info_buffer
// %r ... 
EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE* system_table) {
  Print(L"Welcome to Chan OS !!\n");

  CHAR8 memmap_buf[4096 * 4];
  struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
  GetMemoryMap(&memmap);

  EFI_FILE_PROTOCOL* root_dir;
  OpenRootDir(image_handle, &root_dir);

  EFI_FILE_PROTOCOL* memmap_file;
  root_dir->Open(
      root_dir, &memmap_file, L"\\memmap",
      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);

  SaveMemoryMap(&memmap, memmap_file);
  memmap_file->Close(memmap_file);

  // #@@range_begin(read_kernel)
  EFI_FILE_PROTOCOL* kernel_file;
  root_dir->Open(
      root_dir, &kernel_file, L"\\kernel.elf",
      EFI_FILE_MODE_READ, 0);

  UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12;
  UINT8 file_info_buffer[file_info_size];
  kernel_file->GetInfo(
      kernel_file, &gEfiFileInfoGuid,
      &file_info_size, file_info_buffer);

  EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
  UINTN kernel_file_size = file_info->FileSize;

  EFI_PHYSICAL_ADDRESS kernel_base_addr = 0x100000;
  gBS->AllocatePages(
      AllocateAddress, EfiLoaderData,
      (kernel_file_size + 0xfff) / 0x1000, &kernel_base_addr);
  kernel_file->Read(kernel_file, &kernel_file_size, (VOID*)kernel_base_addr);
  Print(L"Kernel: 0x%0lx (%lu bytes)\n", kernel_base_addr, kernel_file_size);
  // #@@range_end(read_kernel)

  // #@@range_begin(exit_bs)
  EFI_STATUS status;
  status = gBS->ExitBootServices(image_handle, memmap.map_key);
  if (EFI_ERROR(status)) {
    status = GetMemoryMap(&memmap);
    if (EFI_ERROR(status)) {
      Print(L"failed to get memory map: %r\n", status);
      while(1);
    }
    status = gBS->ExitBootServices(image_handle, memmap.map_key);
    if (EFI_ERROR(status)) {
      Print(L"Could not exit boot services: %r\n", status);
      while(1);
    }
  }
  // #@@range_end(exit_bs)

  // #@@range_begin(call_kernel)
  UINT64 entry_addr = *(UINT64*)(kernel_base_addr + 24);

  typedef void EntryPointType(void);
  EntryPointType* entry_point = (EntryPointType*)entry_addr;
  entry_point();
  // #@@range_end(call_kernel)

  Print(L"All done\n");

  while(1);
  return EFI_SUCCESS;
}
