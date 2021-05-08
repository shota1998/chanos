#include <Uefi.h>
#include <Library/UefiLib.h>

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
  UINTN desciptor_size;
  UINT32 desciptor_version;
};

EFI_STATUS GetMemoryMap(struct MemoryMap* map) {
  if (map->buffer == NULL) {
    return EFI_BUFFER_TOO_SMALL;
  }

  map->map_size = map->buffer_size;
  return gBS->GetMemoryMap(
      &map->map_size,
      (EFI_MEMORY_DESCRIPTER*)map->buffer,
      &map->map_key,
      &map->descriptor_size,
      &map->descriptor_version);
}

// AsciiStrLen() ... Returns the length of a Null-terminated ASCII string.
// EFI_PHYSICAL_ADDRESS ... 64-bit physical memory address.
// AsciiSPrint ... Like C's "sprintf".

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
	for ( iter = (EFI_PHISICAL_ADDRESS)map->buffer, i = 0;
	      iter < (EFI_PHISICAL_ADDRESS)map->buffer + map->map_size;
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

// EFIAPI ...  
// EFI_HANDLE ... ImageHandle refers to the image handle of the UEFI application. 
// EFI_SYSTEM_TABLE ... SystemTable is the pointer to the EFI System Table.
//  L ... Encode wide characters with UCS2.
//
// EFI_FILE_PROTOCOL ... Provides file IO access to supported file systems.
// root_dir->Open ... Make "memmap" file at root dir.
// SaveMemoryMap ... Save memmap to " ~/memmap ".

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

  Print(L"All done\n");

  while(1);
  return EFI_SUCCESS;
}
