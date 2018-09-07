
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int  u32;

u8 g_packed[] = {
    0x02,
    0x01,
    0x02,
    0x02,
    0x80,
    0x03,
    0x81,
    0x04,
    0x82,
    0x95
};

u8 g_unpacked[] = {
    0x01, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x5, 0x76, 0x76, 0x13, 0x65, 0x95, 0x95, 0x95
};

//01 02 02 03 03 03 04 04 04 04 95 95 95 95 95

u32 pack_bits(u8* data, u32 length, u8* allocated_array) {
    
    u32 data_index        = 0;
    u32 packed_data_index = 0;
    
    while(data_index < length) {
        
        u8 curr_byte  = data[data_index];
        
        printf("%02x repeater: 0\n", curr_byte);
        
        u8 repeater  = 0;
        
        u16 i;
        
        //scan section (128 bytes)
        for(i = 1; i < 0x80 && data_index + i < length; i++) {
            
            printf("%06u.: 0x%02x ", data_index + i, data[data_index + i]);
            
            if(curr_byte != data[data_index + i]) {
                
                if(repeater > 1) {
                    
                    //pack non repeated section
                    if((i - (repeater + 1)) != 0) {
                        printf("[pack normal section: %u with length: %u] ", data_index, i - (repeater + 1));
                        
                        printf("write: %06u byte: 0x%02x\n", packed_data_index, i - (repeater + 1) - 1);
                        
                        allocated_array[packed_data_index] = i - (repeater + 1) - 1;
                        
                        memcpy(allocated_array + packed_data_index + 1, data + data_index, i - (repeater + 1));
                        
                        packed_data_index += i - (repeater + 1) + 1;
                    }
                    
                    //pack repeated section
                    printf("[pack repeater section: %u with %u duplicates]\n", (data_index + i - repeater) - 1, repeater + 1);
                    
                    printf("write: %06u byte: 0x%02x\n", packed_data_index, 0x80 + repeater + 1);
                    printf("write: %06u byte: 0x%02x\n", packed_data_index, curr_byte);
                    
                    allocated_array[packed_data_index]     = 0x80 + repeater + 1 - 3;
                    allocated_array[packed_data_index + 1] = curr_byte;
                    
                    packed_data_index += 2;
                    
                    curr_byte = data[data_index + i];
                    repeater  = 0;
                    break;
                }
                
                curr_byte = data[data_index + i];
                repeater  = 0;
                
            } else {
                
                repeater++;
                
            }
            
            printf("\n");
            
        }
        
        /*TODO: this is all wrong you bitch
                well it works but only if the last saction is not repeater section :/
         */
        if(data_index + i == length) {
            
            printf("[pack normal section: %u with length: %u] ", data_index, i - (repeater + 1));
            
            printf("write: %06u byte: 0x%02x\n", packed_data_index, i - (repeater + 1) - 1);
            
            allocated_array[packed_data_index] = i - (repeater + 1) - 1;
            
            memcpy(allocated_array + packed_data_index + 1, data + data_index, i - (repeater + 1));
            
            packed_data_index += i - (repeater + 1);
            
            if(repeater > 1) {
                printf("[pack repeater section: %u with %u duplicates]\n", (data_index + i - repeater) - 1, repeater + 1);
                
                allocated_array[packed_data_index]     = 0x80 + repeater + 1 - 3;
                allocated_array[packed_data_index + 1] = curr_byte;
                
                packed_data_index += 2;
            }
        }
        
        data_index    += i;
    }
    
    return packed_data_index;
}

u32 unpack_bits(u8* data, u32 length, u8* allocated_array) {
    
    u32 unpacked_index = 0;
    
    for(u32 i = 0; i < length; i++) {
        
        if(data[i] < 0x80) {
            
            memcpy(allocated_array + unpacked_index,
                   data + i + 1,
                   data[i]  + 1);
            
            
            unpacked_index += data[i] + 1;
            i              += data[i] + 1;
            
        } else {
            
            for(u32 j = 0; j < (data[i] - 0x80 + 3); j++) {
                allocated_array[unpacked_index++] = data[i + 1];
            }
            
            i += 1;
            
        }
    }
    
    return unpacked_index;
}

int main(int argc, char* argv[]) {

    FILE* in = fopen("/users/silent/desktop/text.txt", "rb");
    
    fseek(in, 0, SEEK_END);
    u32 in_size = ftell(in);
    fseek(in, 0, SEEK_SET);
    
    u8* in_buffer    = calloc(in_size + 1, 1);
    u8* check_buffer = calloc(in_size + 1, 1);
    u8* out_buffer   = malloc(in_size * 1.2);
    
    fread(in_buffer, 1, in_size, in);
    
    u32 packed_length   = pack_bits(in_buffer, in_size, out_buffer);
    
    u32 unpacked_length = unpack_bits(out_buffer, packed_length, check_buffer);
    
    
    if(unpacked_length != in_size) {
        printf("original: %u\nnew:      %u\npacked:   %u\n", in_size, unpacked_length, packed_length);
    }
    
    printf("packed:\n");
    for(u32 i = 0; i < packed_length; i++) {
        printf("0x%02x\n", out_buffer[i]);
    }
    
    printf("comparasion:\n");
    for(u32 i = 0; i < in_size; i++) {
        printf("0x%02x %s 0x%02x\n", in_buffer[i], in_buffer[i] == check_buffer[i] ? "=" : "\\", check_buffer[i]);
    }
    
    
    
    
    free(in_buffer);
    fclose(in);
    
    /*
     *DEBUKVA
     *
    printf("unpacked:   ");
    for(u32 i = 0; i < sizeof(g_unpacked); i++) {
        printf("%02x ", g_unpacked[i]);
    }
    printf("\n");
    
    u8 packed[100]   = { 0 };
    u8 unpacked[100] = { 0 };
    
    u32 packed_length = pack_bits(g_unpacked, sizeof(g_unpacked), packed);
    
    printf("packed:     ");
    for(u32 i = 0; i < packed_length; i++) {
        printf("%02x ", packed[i]);
    }
    printf("\n");
    
    u32 unpacked_length = unpack_bits(packed, packed_length, unpacked);
    
    printf("reunpacked: ");
    for(u32 i = 0; i < unpacked_length; i++) {
        printf("%02x ", unpacked[i]);
    }
    printf("\n");
     */
}
