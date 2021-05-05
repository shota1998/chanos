#include <Uefi.h>
#include <Library/UefiLib.h>

// EFIAPI ...  
// EFI_HANDLE ... ImageHandle refers to the image handle of the UEFI application. 
// EFI_SYSTEM_TABLE ... SystemTable is the pointer to the EFI System Table.
//  L ... Encode wide characters with UCS2.

EFI_STATUS EFIAPI UefiMain(
    EFI_HANDLE image_handle,
    EFI_SYSTEM_TABLE *system_table) {
  Print(L"Welcome to Chan OS !!\n");
  while(1);
  return EFI_SUCCESS;
}
