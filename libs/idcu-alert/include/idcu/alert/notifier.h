#ifndef IDCU_ALERT_NOTIFIER_H
#define IDCU_ALERT_NOTIFIER_H

#include "idcu/alert/alert.h"
#include "idcu/common/error_code.h"

#include <stddef.h>
#include <stdint.h>

#define IDCU_NOTIFIER_MAX_CHANNELS         16
#define IDCU_NOTIFIER_WEBHOOK_URL_MAX      1024
#define IDCU_NOTIFIER_FILE_PATH_MAX        512
#define IDCU_NOTIFIER_DINGTALK_WEBHOOK_MAX 1024
#define IDCU_NOTIFIER_WECHAT_WEBHOOK_MAX   1024
#define IDCU_NOTIFIER_EMAIL_ADDR_MAX       256
#define IDCU_NOTIFIER_SMTP_HOST_MAX        256

typedef enum
{
    IDCU_NOTIFIER_CHANNEL_LOG = 0,
    IDCU_NOTIFIER_CHANNEL_FILE,
    IDCU_NOTIFIER_CHANNEL_WEBHOOK,
    IDCU_NOTIFIER_CHANNEL_DINGTALK,
    IDCU_NOTIFIER_CHANNEL_WECHAT,
    IDCU_NOTIFIER_CHANNEL_EMAIL
} idcu_NotifierChannelType;

typedef struct
{
    char webhook_url[IDCU_NOTIFIER_DINGTALK_WEBHOOK_MAX];
    char secret[256];
    int  at_all;
    char at_mobiles[1024];
} idcu_DingtalkConfig;

typedef struct
{
    char webhook_url[IDCU_NOTIFIER_WECHAT_WEBHOOK_MAX];
    int  at_all;
    char at_userids[1024];
} idcu_WechatConfig;

typedef struct
{
    char smtp_host[IDCU_NOTIFIER_SMTP_HOST_MAX];
    int  smtp_port;
    char from_addr[IDCU_NOTIFIER_EMAIL_ADDR_MAX];
    char to_addrs[IDCU_NOTIFIER_EMAIL_ADDR_MAX * 4];
    char username[256];
    char password[256];
    int  use_tls;
} idcu_EmailConfig;

typedef struct
{
    idcu_NotifierChannelType type;
    int                      enabled;
    union
    {
        struct
        {
            char file_path[IDCU_NOTIFIER_FILE_PATH_MAX];
        } file;
        struct
        {
            char url[IDCU_NOTIFIER_WEBHOOK_URL_MAX];
            int  timeout_ms;
        } webhook;
        idcu_DingtalkConfig dingtalk;
        idcu_WechatConfig   wechat;
        idcu_EmailConfig    email;
    } config;
} idcu_NotifierChannel;

typedef struct
{
    idcu_NotifierChannel channels[IDCU_NOTIFIER_MAX_CHANNELS];
    int                  channel_count;
    int                  initialized;
} idcu_Notifier;

int  idcu_notifier_init(idcu_Notifier* notifier);
void idcu_notifier_destroy(idcu_Notifier* notifier);

int idcu_notifier_add_log_channel(idcu_Notifier* notifier);
int idcu_notifier_add_file_channel(idcu_Notifier* notifier, const char* file_path);
int idcu_notifier_add_webhook_channel(idcu_Notifier* notifier, const char* url, int timeout_ms);
int idcu_notifier_add_dingtalk_channel(idcu_Notifier* notifier, const char* webhook_url,
                                       const char* secret);
int idcu_notifier_add_wechat_channel(idcu_Notifier* notifier, const char* webhook_url);
int idcu_notifier_add_email_channel(idcu_Notifier* notifier, const char* smtp_host, int smtp_port,
                                    const char* from_addr, const char* to_addrs,
                                    const char* username, const char* password, int use_tls);

int idcu_notifier_send(idcu_Notifier* notifier, const idcu_AlertEvent* event);
int idcu_notifier_set_channel_enabled(idcu_Notifier* notifier, int index, int enabled);
int idcu_notifier_remove_channel(idcu_Notifier* notifier, int index);
int idcu_notifier_clear_channels(idcu_Notifier* notifier);

void idcu_notifier_alert_callback(const idcu_AlertEvent* event, void* user_data);

const char* idcu_notifier_channel_type_to_string(idcu_NotifierChannelType type);

#endif
