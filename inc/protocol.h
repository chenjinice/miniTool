#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <QString>
#include <QList>

#define MSG_IPDATA_SIZE 18
#define MSG_VERSION_SIZE 8
#define MSG_FBODY_SIZE  1200
#define MSG_RSIZE           10
#define MSG_UPDATE_RSIZE    12
#define MSG_IPVALID_RSIZE   32
#define MSG_IP_RSIZE        MSG_IPVALID_RSIZE+9
#define MSG_LEDVALID_RSIZE  9
#define MSG_LED_RSIZE       9+9

typedef enum{
    PROTOCOL_OK,
    PROTOCOL_FAIL,
    PROTOCOL_ERROR
}Protocol_err;

typedef struct{
    uint8_t direction;
    uint8_t position;
    uint8_t color;
    uint8_t time;
}Led_info;

uint8_t check_sum(uint8_t *p, int length);
bool    ip_to_array(const char *str, int *array);
bool    mac_to_array(const char *str, int *array);
uint8_t file_pack_num(int length);

void get_ip_data(uint8_t *p,int *ip,int * mask,int * gateway,int * mac);
int protocol_getip(uint8_t *p);
int protocol_setip(uint8_t *p, uint8_t *ip_data);
int protocol_lightStatus(uint8_t *p);
int protocol_reboot(uint8_t *p);
int protocol_update(uint8_t *p, uint8_t *bin, int bin_size, uint8_t page, int *send_size);
int protocol_preUpdate(uint8_t *p,uint8_t *version);
int protocol_endUpdate(uint8_t *p);


Protocol_err protocol_check_setip(uint8_t *p ,int length);
Protocol_err protocol_check_getip(uint8_t *p , int length, int *index);
Protocol_err protocol_check_reboot(uint8_t *p ,int length);
Protocol_err protocol_check_update(uint8_t *p , int length);
Protocol_err protocol_check_preUpdate(uint8_t *p ,int length);
Protocol_err protocol_check_endUpdate(uint8_t *p ,int length);
QList<Led_info> protocol_check_lightStatus(uint8_t *p ,int length);


int protocol_direct(Led_info &info);
QString protocol_direct_String(Led_info &info);

#endif // PROTOCOL_H
