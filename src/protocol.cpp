#include "inc/protocol.h"
#include <stdio.h>
#include <QDebug>

#define MSG_HEAD1   0xa5
#define MSG_HEAD2   0x5a
#define MSG_TAIL1   0x5a
#define MSG_TAIL2   0xa5


uint8_t check_sum(uint8_t *p, int length)
{
    uint8_t sum = 0;

    if(p == nullptr)return sum;

    for(int i=0;i<length;i++)
    {
        sum += p[i];
    }
    return sum;
}

bool ip_to_array(const char *str, int *array)
{
    bool ret = false;
    if(str == nullptr)return ret;
    if(array == nullptr)return  ret;

    int count =sscanf(str,"%d.%d.%d.%d",array,array+1,array+2,array+3);
    if(count !=4)return ret;

    for(int i=0;i<count;i++)
    {
        if(array[i] > 255 || array[i] < 0)return ret;
    }

    ret = true;
    return ret;
}

bool mac_to_array(const char *str, int *array)
{
    bool ret = false;
    if(str == nullptr)return ret;
    if(array == nullptr)return  ret;

    int count =sscanf(str,"%8x:%8x:%8x:%8x:%8x:%8x",array,array+1,array+2,array+3,array+4,array+5);
    if(count !=6)return ret;

    for(int i=0;i<count;i++)
    {
        if(array[i] > 255 || array[i] < 0)return ret;
    }

    ret = true;
    return ret;
}

void get_ip_data(uint8_t *p, int *ip, int *mask, int *gateway, int *mac)
{
    int count = 0;
    for(int i=0;i<4;i++){
        p[count++] = static_cast<uint8_t>(ip[i]);
    }
    for(int i=0;i<4;i++){
        p[count++] = static_cast<uint8_t>(mask[i]);
    }
    for(int i=0;i<4;i++){
        p[count++] = static_cast<uint8_t>(gateway[i]);
    }
    for(int i=0;i<6;i++){
        p[count++] = static_cast<uint8_t>(mac[i]);
    }
}


// ---------------- send cmd ------------------
int protocol_getip(uint8_t *p)
{
    int count = 0;
    uint8_t sum = 0;
    if(p == nullptr)return count;
    p[count++] = MSG_HEAD1;
    p[count++] = MSG_HEAD2;
    p[count++] = 0x83;
    p[count++] = 0xa0;
    p[count++] = 0x01;
    sum= check_sum(p,count);
    p[count++] = sum;
    p[count++] = MSG_TAIL1;
    p[count++] = MSG_TAIL2;
    return count;
}

int protocol_setip(uint8_t *p, uint8_t *ip_data)
{
    int count = 0;
    uint8_t sum = 0;
    if(p == nullptr)return count;
    if(ip_data == nullptr)return count;
    p[count++] = MSG_HEAD1;
    p[count++] = MSG_HEAD2;
    p[count++] = 0x83;
    p[count++] = 0xa0;
    p[count++] = 0x02;

    p[count++] = MSG_IPDATA_SIZE;
    for(int i=0;i<MSG_IPDATA_SIZE;i++)
    {
        p[count++] = ip_data[i];
    }
    sum= check_sum(p,count);
    p[count++] = sum;
    p[count++] = MSG_TAIL1;
    p[count++] = MSG_TAIL2;
    return count;
}

int protocol_lightStatus(uint8_t *p)
{
    int count = 0;
    uint8_t sum = 0;
    if(p == nullptr)return count;
    p[count++] = MSG_HEAD1;
    p[count++] = MSG_HEAD2;
    p[count++] = 0x83;
    p[count++] = 0xa0;
    p[count++] = 0x00;
    sum= check_sum(p,count);
    p[count++] = sum;
    p[count++] = MSG_TAIL1;
    p[count++] = MSG_TAIL2;
    return count;
}

int protocol_reboot(uint8_t *p)
{
    int count = 0;
    uint8_t sum = 0;
    if(p == nullptr)return count;
    p[count++] = MSG_HEAD1;
    p[count++] = MSG_HEAD2;
    p[count++] = 0x83;
    p[count++] = 0xa0;
    p[count++] = 0x03;
    sum= check_sum(p,count);
    p[count++] = sum;
    p[count++] = MSG_TAIL1;
    p[count++] = MSG_TAIL2;
    return count;
}

int protocol_update(uint8_t *p, uint8_t *bin, int bin_size, uint8_t page,int *s_size)
{
    int count = 0;
    uint8_t sum = 0;
    if(p == nullptr)return count;
    p[count++] = MSG_HEAD1;
    p[count++] = MSG_HEAD2;
    p[count++] = 0x83;
    p[count++] = 0xa0;
    p[count++] = 0x05;

    int bin_index = page*MSG_FBODY_SIZE;
    if(bin_index >= bin_size)return 0;
    int bin_send_length = 0;
    if( (bin_index+MSG_FBODY_SIZE) > bin_size ){
        bin_send_length = bin_size - bin_index ;
    }else{
        bin_send_length = MSG_FBODY_SIZE;
    }

    p[count++] = bin_send_length & 0xff;
    p[count++] = (bin_send_length >> 8) & 0xff;
    p[count++] = page+1;

    for(int i=0;i<bin_send_length;i++)
    {
        p[count++] = bin[bin_index+i];
    }

//    qDebug() << bin_send_length << "========";

    sum= check_sum(p,count);
    p[count++] = sum;
    p[count++] = MSG_TAIL1;
    p[count++] = MSG_TAIL2;

    *s_size = 0;
    *s_size = bin_send_length;
    return count;
}

int protocol_preUpdate(uint8_t *p, uint8_t *version)
{
    int count = 0;
    uint8_t sum = 0;
    if(p == nullptr)return count;
    p[count++] = MSG_HEAD1;
    p[count++] = MSG_HEAD2;
    p[count++] = 0x83;
    p[count++] = 0xa0;
    p[count++] = 0x04;

    p[count++] = MSG_VERSION_SIZE;
    for(int i=0;i<MSG_VERSION_SIZE;i++){
        p[count++] = version[i];
    }

    sum= check_sum(p,count);
    p[count++] = sum;
    p[count++] = MSG_TAIL1;
    p[count++] = MSG_TAIL2;
    return count;
}

int protocol_endUpdate(uint8_t *p)
{
    int count = 0;
    uint8_t sum = 0;
    if(p == nullptr)return count;
    p[count++] = MSG_HEAD1;
    p[count++] = MSG_HEAD2;
    p[count++] = 0x83;
    p[count++] = 0xa0;
    p[count++] = 0x06;
    sum= check_sum(p,count);
    p[count++] = sum;
    p[count++] = MSG_TAIL1;
    p[count++] = MSG_TAIL2;
    return count;
}



// -----------------
Protocol_err protocol_check_setip(uint8_t *p, int length)
{
    uint8_t sum = 0;
    int count = 0;
    Protocol_err p_ret = PROTOCOL_ERROR;
    if(p == nullptr)return PROTOCOL_ERROR;
    if(length < MSG_RSIZE)return PROTOCOL_ERROR;

    for(int i=0;i<=length-MSG_RSIZE;i++)
    {
        count = i;
        if(p[count++] != MSG_HEAD1)continue;
        if(p[count++] != MSG_HEAD2)continue;
        if(p[count++] != 0x83)continue;
        if(p[count++] != 0xa0)continue;
        if(p[count++] != 0x02)continue;

        if(p[count++] != 1)continue;
        if(p[count++] == 1)p_ret = PROTOCOL_OK;
        else p_ret = PROTOCOL_FAIL;

        sum= check_sum(p+i,count-i);
        if(p[count++] != sum)continue;
        if(p[count++] != MSG_TAIL1)continue;
        if(p[count++] != MSG_TAIL2)continue;

        return p_ret;
    }
    return PROTOCOL_ERROR;
}

Protocol_err protocol_check_getip(uint8_t *p, int length, int *index)
{
    uint8_t sum = 0;
    int count = 0,temp = 0;
    Protocol_err p_ret = PROTOCOL_ERROR;
    if(p == nullptr)return PROTOCOL_ERROR;
    if(length < MSG_IP_RSIZE)return PROTOCOL_ERROR;


    for(int i=0;i<=length-MSG_IP_RSIZE;i++)
    {
        count = i;
        if(p[count++] != MSG_HEAD1)continue;
        if(p[count++] != MSG_HEAD2)continue;
        if(p[count++] != 0x83)continue;
        if(p[count++] != 0xa0)continue;
        if(p[count++] != 0x01)continue;

        if(p[count++] != MSG_IPVALID_RSIZE)continue;

        temp = count;
        count += MSG_IPVALID_RSIZE;
        sum= check_sum(p+i,count-i);
        if(p[count++] != sum)continue;
        if(p[count++] != MSG_TAIL1)continue;
        if(p[count++] != MSG_TAIL2)continue;

        p_ret = PROTOCOL_OK;
        *index = temp;
//        qDebug("%d : %02x",index,**(buffer));
        return p_ret;
    }
    return PROTOCOL_ERROR;
}

Protocol_err protocol_check_reboot(uint8_t *p, int length)
{
    uint8_t sum = 0;
    int count = 0;
    Protocol_err p_ret = PROTOCOL_ERROR;
    if(p == nullptr)return PROTOCOL_ERROR;
    if(length < MSG_RSIZE)return PROTOCOL_ERROR;

    for(int i=0;i<=length-MSG_RSIZE;i++)
    {
        count = i;
        if(p[count++] != MSG_HEAD1)continue;
        if(p[count++] != MSG_HEAD2)continue;
        if(p[count++] != 0x83)continue;
        if(p[count++] != 0xa0)continue;
        if(p[count++] != 0x03)continue;

        if(p[count++] != 1)continue;
        if(p[count++] == 1)p_ret = PROTOCOL_OK;
        else p_ret = PROTOCOL_FAIL;

        sum= check_sum(p+i,count-i);
        if(p[count++] != sum)continue;
        if(p[count++] != MSG_TAIL1)continue;
        if(p[count++] != MSG_TAIL2)continue;

        return p_ret;
    }
    return PROTOCOL_ERROR;
}

QList<Led_info> protocol_check_lightStatus(uint8_t *p, int length)
{
    uint8_t sum = 0;
    int count = 0;
    QList<Led_info> list;
    if(p == nullptr)return list;
    if(length < MSG_LED_RSIZE)return list;

    for(int i=0;i<=length-MSG_LED_RSIZE;i++)
    {
        int index = 0;
        count = i;
        if(p[count++] != MSG_HEAD1)continue;
        if(p[count++] != MSG_HEAD2)continue;
        if(p[count++] != 0x83)continue;
        if(p[count++] != 0xa0)continue;
        if(p[count++] != 0x00)continue;

        //板子里边写错了数字，不方便动，只能我这边改一下
        if(p[count++] != MSG_LED_RSIZE)continue;
        index = count;
        count += MSG_LEDVALID_RSIZE;

        sum= check_sum(p+i,count-i);
        if(p[count++] != sum)continue;
        if(p[count++] != MSG_TAIL1)continue;
        if(p[count++] != MSG_TAIL2)continue;

        Led_info led;
        led.direction = p[index++];
        led.position = p[index++];
        led.color = p[index++];
        led.time = p[index];
        list.append(led);
    }
    return list;
}

Protocol_err protocol_check_update(uint8_t *p, int length)
{
    uint8_t sum = 0;
    int count = 0;
    Protocol_err p_ret = PROTOCOL_ERROR;
    if(p == nullptr)return PROTOCOL_ERROR;
    if(length < MSG_RSIZE)return PROTOCOL_ERROR;

//    uint8_t low = static_cast<uint8_t>(send_size & 0xff);
//    uint8_t high = static_cast<uint8_t>( (send_size>>8) & 0xff );
//    qDebug("page = %02x,l=%02x,h=%02x",page+1,low,high);

    for(int i=0;i<=length-MSG_RSIZE;i++)
    {
        count = i;
        if(p[count++] != MSG_HEAD1)continue;
        if(p[count++] != MSG_HEAD2)continue;
        if(p[count++] != 0x83)continue;
        if(p[count++] != 0xa0)continue;
        if(p[count++] != 0x05)continue;

//        if(p[count++] != low)continue;
//        if(p[count++] != high)continue;
        if(p[count++] != 1)continue;
//        if(p[count++] != (page+1) )continue;
        if(p[count++] == 1)p_ret = PROTOCOL_OK;
        else p_ret = PROTOCOL_FAIL;

        sum= check_sum(p+i,count-i);
        if(p[count++] != sum)continue;
        if(p[count++] != MSG_TAIL1)continue;
        if(p[count++] != MSG_TAIL2)continue;

        p_ret = PROTOCOL_OK;
        return p_ret;
    }
    return PROTOCOL_ERROR;
}

Protocol_err protocol_check_preUpdate(uint8_t *p, int length)
{
    uint8_t sum = 0;
    int count = 0;
    Protocol_err p_ret = PROTOCOL_ERROR;
    if(p == nullptr)return PROTOCOL_ERROR;
    if(length < MSG_RSIZE)return PROTOCOL_ERROR;

    for(int i=0;i<=length-MSG_RSIZE;i++)
    {
        count = i;
        if(p[count++] != MSG_HEAD1)continue;
        if(p[count++] != MSG_HEAD2)continue;
        if(p[count++] != 0x83)continue;
        if(p[count++] != 0xa0)continue;
        if(p[count++] != 0x04)continue;

        if(p[count++] != 1)continue;
        if(p[count++] == 1)p_ret = PROTOCOL_OK;
        else p_ret = PROTOCOL_FAIL;

        sum= check_sum(p+i,count-i);
        if(p[count++] != sum)continue;
        if(p[count++] != MSG_TAIL1)continue;
        if(p[count++] != MSG_TAIL2)continue;

        return p_ret;
    }
    return PROTOCOL_ERROR;
}

Protocol_err protocol_check_endUpdate(uint8_t *p, int length)
{
    uint8_t sum = 0;
    int count = 0;
    Protocol_err p_ret = PROTOCOL_ERROR;
    if(p == nullptr)return PROTOCOL_ERROR;
    if(length < MSG_RSIZE)return PROTOCOL_ERROR;

    for(int i=0;i<=length-MSG_RSIZE;i++)
    {
        count = i;
        if(p[count++] != MSG_HEAD1)continue;
        if(p[count++] != MSG_HEAD2)continue;
        if(p[count++] != 0x83)continue;
        if(p[count++] != 0xa0)continue;
        if(p[count++] != 0x06)continue;

        if(p[count++] != 1)continue;
        if(p[count++] == 1)p_ret = PROTOCOL_OK;
        else p_ret = PROTOCOL_FAIL;

        sum= check_sum(p+i,count-i);
        if(p[count++] != sum)continue;
        if(p[count++] != MSG_TAIL1)continue;
        if(p[count++] != MSG_TAIL2)continue;

        return p_ret;
    }
    return PROTOCOL_ERROR;
}

uint8_t file_pack_num(int length)
{
    int count = length/MSG_FBODY_SIZE;
    int remainder = length%MSG_FBODY_SIZE;

    if(remainder != 0)count++;

    return  static_cast<uint8_t>(count);
}


int protocol_direct(Led_info &info)
{
    int m = 0 , n = 0 , ret = -1;

    if(info.direction < 4)m = info.direction;
    else return ret;

    if(info.position < 4)n = info.position;
    else return ret;

    ret = m*3 + n -1;
    if(ret >= 12)ret = -1;

    return  ret;
}

QString protocol_direct_String(Led_info &info)
{
    static QStringList s_list = {"东←","东↑","东→","西←","西↑","西→""南←","南↑","南→","北←","北↑","北→"};
    int length = s_list.length();
    QString str;
    int ret = protocol_direct(info);

    if( (ret >= 0) && (ret < length) )str = s_list[ret];

    return  str;
}


