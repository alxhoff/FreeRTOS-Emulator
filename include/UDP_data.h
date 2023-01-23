#ifndef __UDP_DATA_H__
#define __UDP_DATA_H__

struct __attribute__((packed)) data_packet {
    int my_int;
    char my_string[20];
};

#endif // __UDP_DATA_H__