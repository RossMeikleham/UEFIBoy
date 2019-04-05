#include "../../non_core/graphics_out.h"

#include <Uefi.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Library/UefiRuntimeLib.h>

#include <stdlib.h>
#include <string.h>

#define GB_RES_X 160
#define GB_RES_Y 144

static EFI_GRAPHICS_OUTPUT_BLT_PIXEL *pixels;
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL *old_pixels; // Current screen before running the emulator
static EFI_GRAPHICS_OUTPUT_PROTOCOL  *GraphicsOutput = NULL;
static EFI_GRAPHICS_OUTPUT_BLT_PIXEL *output_pixels = NULL;
static int screen_width = 0;
static int screen_height = 0;

extern int scale;
extern int full_size;

static int current_scale = 1;
static int x_res = 0;
static int y_res = 0;

/* Initialize graphics
 * returns 1 if successful, 0 otherwise */
int init_screen(int win_x, int win_y, uint32_t *p) {
 
	UINTN HandleCount;
	EFI_HANDLE  *HandleBuffer = NULL;
   
    scale = 1; 
    current_scale = scale;
    screen_width = win_x;
    screen_height = win_y;

    pixels = (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *)p;

   // Locate all instances of GOP
   EFI_STATUS Status = gBS->LocateHandleBuffer(ByProtocol
	, &gEfiGraphicsOutputProtocolGuid, NULL, &HandleCount, &HandleBuffer);

   if (EFI_ERROR (Status)) {
		Print(L"ShowStatus: Graphics output protocol not found\n");
		return 0;
	}
  	
    Print(L"Graphics output protocols found!\n");
	
	for (UINTN i = 0; i < HandleCount; i++) {
		Status = gBS->HandleProtocol(HandleBuffer[i]
			, &gEfiGraphicsOutputProtocolGuid, (VOID **) &GraphicsOutput);
        
		if (EFI_ERROR (Status)) {
            Print(L"ShowStatus: gBS->HandleProtocol[%d] returned %r\n", i, Status);
            continue;
        } else {

            UINTN size = 0;
            EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
            Status = GraphicsOutput->QueryMode(GraphicsOutput, GraphicsOutput->Mode->Mode, &size, &info);
            info = (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *)malloc(size);
            
            if (info == NULL) {
               continue; 
            }
            
            Status = GraphicsOutput->QueryMode(GraphicsOutput, GraphicsOutput->Mode->Mode, &size, &info);
            if (EFI_ERROR(Status)) {
                free(info);
                Print(L"Unable to get graphics mode info\n");
                continue;
            }

            x_res = info->HorizontalResolution;
            y_res = info->VerticalResolution;

            old_pixels = malloc(x_res * y_res * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
            if (old_pixels != NULL) {
		        GraphicsOutput->Blt(GraphicsOutput, old_pixels, EfiBltVideoToBltBuffer, 0, 0, 0, 0, x_res, y_res, 0); 
            }

            free(info);
            return 1;
        }

    }
	    
    return 0; // Failed
}

void draw_screen() {
        
        if (scale != current_scale) {
            // If resolution is too big/too small
            if (scale < 1 || (GB_RES_X * scale > x_res) || GB_RES_Y * scale > y_res) {
                scale = current_scale;
            } else {
                if (old_pixels != NULL) {
		            GraphicsOutput->Blt(GraphicsOutput, old_pixels, EfiBltBufferToVideo, 0, 0, 0, 0, x_res, y_res, 0); 
                }

                if (output_pixels != NULL) {
                    free(output_pixels);
                }
                output_pixels = malloc(GB_RES_X * GB_RES_Y * scale * scale * sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));
                current_scale = scale;
            }
        }

        UINTN x_offset = (x_res - (scale * GB_RES_X)) / 2;
        UINTN y_offset = (y_res - (scale * GB_RES_Y)) / 2;
      
        if (current_scale <= 1) {
		    GraphicsOutput->Blt(GraphicsOutput, pixels, EfiBltBufferToVideo, 0, 0, x_offset, y_offset, GB_RES_X, GB_RES_Y, 0); 
        } else {

            uint32_t *frame_buffer = (uint32_t *)output_pixels;
            
            for (UINTN y_pix = 0; y_pix < GB_RES_Y; y_pix++) {
                uint32_t *frame_buffer_row = ((uint32_t *)output_pixels) + (y_pix * current_scale * current_scale * GB_RES_X);
                for (UINTN y_pix_scale = 0; y_pix_scale < current_scale; y_pix_scale++) {
                    frame_buffer = frame_buffer_row + (GB_RES_X * scale * y_pix_scale);
                    uint32_t *current_pix = ((uint32_t *)pixels) + (y_pix * GB_RES_X);
                    for (UINTN x_pix = 0; x_pix < GB_RES_X; x_pix++, frame_buffer +=current_scale) {
                        for (UINTN i = 0; i < current_scale; i++) {
                            frame_buffer[i] = *(((int*)current_pix) + x_pix);
                        }
                    } 
                }
            }
		    
            GraphicsOutput->Blt(GraphicsOutput, output_pixels, EfiBltBufferToVideo, 0, 0, x_offset, y_offset, GB_RES_X * scale, GB_RES_Y * scale, 0); 
        }
}

void cleanup_graphics_out() {
    // Return screen buffer to state before running the emulator
    if (old_pixels != NULL) {
        GraphicsOutput->Blt(GraphicsOutput, old_pixels, EfiBltBufferToVideo, 0, 0, 0, 0, x_res, y_res, 0); 
        free(old_pixels);
    }
}
