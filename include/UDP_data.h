#ifndef __UDP_DATA_H__
#define __UDP_DATA_H__

#define PACKET_TYPE_CONFIG 0x0
#define PACKET_TYPE_DATA 0x1

// The packed attribute here is important if you want to parse
// your structs manually, ie. struct pointer + so many bytes.
// Packing the structs will ensure the compiler doesn't add in
// padding for memory allignment.
struct __attribute__((packed)) config_packet {
    char packet_type;
    char config_byte;
};

struct __attribute__((packed)) data_packet {
    char packet_type;
    int my_int;
    char my_string[20];
};

#endif // __UDP_DATA_H__