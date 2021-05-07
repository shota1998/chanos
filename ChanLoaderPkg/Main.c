#include <Uefi.h>
#include <Library/UefiLib.h>

// gBS ... Global variable representing the boot service.
// gBS->GetMemoryMap ... Get the memory map at the time of the function call.
//  Write to the memory specified by the MemoryMap argument.
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
  UINTN32 desciptor_version;
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
      &map->descriptot_version);
}


// EFIAPI ...  
// EFI_HANDLE ... ImageHandle refers to the image handle of the UEFI application. 
// EFI_SYSTEM_TABLE ... SystemTable is the pointer to the EFI System Table.
//  L ... Encode wide characters with UCS2.

EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE* system_table) {
  Print(L"Welcome to Chan OS !!\n");
  while(1);
  return EFI_SUCCESS;
}
