#include "idcu/alert/notifier.h"
#include "idcu/log/log.h"
#include <string.h>
#include <stdio.h>
#include <time.h>

static idcu_Notifier* g_notifier_instance = NULL;

static const char* get_level_str(idcu_AlertLevel level)
{
    switch (level) {
        case IDCU_ALERT_LEVEL_INFO: return "INFO";
        case IDCU_ALERT_LEVEL_WARNING: return "WARNING";
        case IDCU_ALERT_LEVEL_ERROR: return "ERROR";
        case IDCU_ALERT_LEVEL_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

static const char* get_state_str(idcu_AlertState state)
{
    switch (state) {
        case IDCU_ALERT_STATE_INACTIVE: return "INACTIVE";
        case IDCU_ALERT_STATE_PENDING: return "PENDING";
        case IDCU_ALERT_STATE_FIRING: return "FIRING";
        case IDCU_ALERT_STATE_RESOLVED: return "RESOLVED";
        case IDCU_ALERT_STATE_SILENCED: return "SILENCED";
        default: return "UNKNOWN";
    }
}

const char* idcu_alert_level_to_string(idcu_AlertLevel level)
{
    return get_level_str(level);
}

const char* idcu_alert_state_to_string(idcu_AlertState state)
{
    return get_state_str(state);
}

const char* idcu_notifier_channel_type_to_string(idcu_NotifierChannelType type)
{
    switch (type) {
        case IDCU_NOTIFIER_CHANNEL_LOG: return "LOG";
        case IDCU_NOTIFIER_CHANNEL_FILE: return "FILE";
        case IDCU_NOTIFIER_CHANNEL_WEBHOOK: return "WEBHOOK";
        case IDCU_NOTIFIER_CHANNEL_DINGTALK: return "DINGTALK";
        case IDCU_NOTIFIER_CHANNEL_WECHAT: return "WECHAT";
        case IDCU_NOTIFIER_CHANNEL_EMAIL: return "EMAIL";
        default: return "UNKNOWN";
    }
}

static void format_alert_message(char* buf, size_t buf_size, const idcu_AlertEvent* event)
{
    time_t t = time(NULL);
    struct tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm);
    
    snprintf(buf, buf_size,
             "[%s] Alert: %s\n"
             "Level: %s\n"
             "State: %s\n"
             "Message: %s\n",
             time_str,
             event->alert_name,
             get_level_str(event->level),
             get_state_str(event->state),
             event->message);
}

static int send_to_log(const idcu_AlertEvent* event)
{
    char msg[2048];
    format_alert_message(msg, sizeof(msg), event);
    
    if (event->state == IDCU_ALERT_STATE_FIRING) {
        switch (event->level) {
            case IDCU_ALERT_LEVEL_CRITICAL:
            case IDCU_ALERT_LEVEL_ERROR:
                IDCU_LOG_ERROR("%s", msg);
                break;
            case IDCU_ALERT_LEVEL_WARNING:
                IDCU_LOG_WARN("%s", msg);
                break;
            default:
                IDCU_LOG_INFO("%s", msg);
        }
    } else if (event->state == IDCU_ALERT_STATE_RESOLVED) {
        IDCU_LOG_INFO("%s", msg);
    }
    
    return IDCU_ERR_SUCCESS;
}

static int send_to_file(const idcu_NotifierChannel* channel, const idcu_AlertEvent* event)
{
    char msg[2048];
    format_alert_message(msg, sizeof(msg), event);
    
    FILE* f = fopen(channel->config.file.file_path, "a");
    if (!f) {
        IDCU_LOG_ERROR("notifier: failed to open file %s", channel->config.file.file_path);
        return IDCU_ERR_FILE_OPEN;
    }
    
    fprintf(f, "%s\n", msg);
    fclose(f);
    
    return IDCU_ERR_SUCCESS;
}

static int send_to_webhook(const idcu_NotifierChannel* channel, const idcu_AlertEvent* event)
{
    (void)channel;
    (void)event;
    IDCU_LOG_WARN("notifier: webhook channel requires HTTP client implementation");
    return IDCU_ERR_SUCCESS;
}

static int send_to_dingtalk(const idcu_NotifierChannel* channel, const idcu_AlertEvent* event)
{
    (void)channel;
    (void)event;
    IDCU_LOG_WARN("notifier: dingtalk channel requires HTTP client and crypto implementation");
    return IDCU_ERR_SUCCESS;
}

static int send_to_wechat(const idcu_NotifierChannel* channel, const idcu_AlertEvent* event)
{
    (void)channel;
    (void)event;
    IDCU_LOG_WARN("notifier: wechat channel requires HTTP client implementation");
    return IDCU_ERR_SUCCESS;
}

static int send_to_email(const idcu_NotifierChannel* channel, const idcu_AlertEvent* event)
{
    (void)channel;
    (void)event;
    IDCU_LOG_WARN("notifier: email channel requires SMTP client implementation");
    return IDCU_ERR_SUCCESS;
}

int idcu_notifier_init(idcu_Notifier* notifier)
{
    if (!notifier) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    memset(notifier, 0, sizeof(idcu_Notifier));
    notifier->initialized = 1;
    g_notifier_instance = notifier;
    
    IDCU_LOG_INFO("notifier initialized");
    return IDCU_ERR_SUCCESS;
}

void idcu_notifier_destroy(idcu_Notifier* notifier)
{
    if (!notifier || !notifier->initialized) {
        return;
    }
    
    notifier->initialized = 0;
    g_notifier_instance = NULL;
    IDCU_LOG_INFO("notifier destroyed");
}

int idcu_notifier_add_log_channel(idcu_Notifier* notifier)
{
    if (!notifier || !notifier->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (notifier->channel_count >= IDCU_NOTIFIER_MAX_CHANNELS) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    idcu_NotifierChannel* ch = &notifier->channels[notifier->channel_count++];
    ch->type = IDCU_NOTIFIER_CHANNEL_LOG;
    ch->enabled = 1;
    
    IDCU_LOG_INFO("notifier: log channel added");
    return IDCU_ERR_SUCCESS;
}

int idcu_notifier_add_file_channel(idcu_Notifier* notifier, const char* file_path)
{
    if (!notifier || !notifier->initialized || !file_path) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (notifier->channel_count >= IDCU_NOTIFIER_MAX_CHANNELS) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    idcu_NotifierChannel* ch = &notifier->channels[notifier->channel_count++];
    ch->type = IDCU_NOTIFIER_CHANNEL_FILE;
    ch->enabled = 1;
    strncpy(ch->config.file.file_path, file_path, sizeof(ch->config.file.file_path) - 1);
    
    IDCU_LOG_INFO("notifier: file channel added: %s", file_path);
    return IDCU_ERR_SUCCESS;
}

int idcu_notifier_add_webhook_channel(idcu_Notifier* notifier, const char* url, int timeout_ms)
{
    if (!notifier || !notifier->initialized || !url) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (notifier->channel_count >= IDCU_NOTIFIER_MAX_CHANNELS) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    idcu_NotifierChannel* ch = &notifier->channels[notifier->channel_count++];
    ch->type = IDCU_NOTIFIER_CHANNEL_WEBHOOK;
    ch->enabled = 1;
    strncpy(ch->config.webhook.url, url, sizeof(ch->config.webhook.url) - 1);
    ch->config.webhook.timeout_ms = timeout_ms;
    
    IDCU_LOG_INFO("notifier: webhook channel added: %s", url);
    return IDCU_ERR_SUCCESS;
}

int idcu_notifier_add_dingtalk_channel(idcu_Notifier* notifier, const char* webhook_url, const char* secret)
{
    if (!notifier || !notifier->initialized || !webhook_url) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (notifier->channel_count >= IDCU_NOTIFIER_MAX_CHANNELS) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    idcu_NotifierChannel* ch = &notifier->channels[notifier->channel_count++];
    ch->type = IDCU_NOTIFIER_CHANNEL_DINGTALK;
    ch->enabled = 1;
    strncpy(ch->config.dingtalk.webhook_url, webhook_url, sizeof(ch->config.dingtalk.webhook_url) - 1);
    if (secret) {
        strncpy(ch->config.dingtalk.secret, secret, sizeof(ch->config.dingtalk.secret) - 1);
    }
    ch->config.dingtalk.at_all = 0;
    ch->config.dingtalk.at_mobiles[0] = '\0';
    
    IDCU_LOG_INFO("notifier: dingtalk channel added");
    return IDCU_ERR_SUCCESS;
}

int idcu_notifier_add_wechat_channel(idcu_Notifier* notifier, const char* webhook_url)
{
    if (!notifier || !notifier->initialized || !webhook_url) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (notifier->channel_count >= IDCU_NOTIFIER_MAX_CHANNELS) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    idcu_NotifierChannel* ch = &notifier->channels[notifier->channel_count++];
    ch->type = IDCU_NOTIFIER_CHANNEL_WECHAT;
    ch->enabled = 1;
    strncpy(ch->config.wechat.webhook_url, webhook_url, sizeof(ch->config.wechat.webhook_url) - 1);
    ch->config.wechat.at_all = 0;
    ch->config.wechat.at_userids[0] = '\0';
    
    IDCU_LOG_INFO("notifier: wechat channel added");
    return IDCU_ERR_SUCCESS;
}

int idcu_notifier_add_email_channel(idcu_Notifier* notifier, const char* smtp_host, int smtp_port,
                                      const char* from_addr, const char* to_addrs,
                                      const char* username, const char* password, int use_tls)
{
    if (!notifier || !notifier->initialized || !smtp_host || !from_addr || !to_addrs) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    if (notifier->channel_count >= IDCU_NOTIFIER_MAX_CHANNELS) {
        return IDCU_ERR_QUEUE_FULL;
    }
    
    idcu_NotifierChannel* ch = &notifier->channels[notifier->channel_count++];
    ch->type = IDCU_NOTIFIER_CHANNEL_EMAIL;
    ch->enabled = 1;
    strncpy(ch->config.email.smtp_host, smtp_host, sizeof(ch->config.email.smtp_host) - 1);
    ch->config.email.smtp_port = smtp_port;
    strncpy(ch->config.email.from_addr, from_addr, sizeof(ch->config.email.from_addr) - 1);
    strncpy(ch->config.email.to_addrs, to_addrs, sizeof(ch->config.email.to_addrs) - 1);
    if (username) {
        strncpy(ch->config.email.username, username, sizeof(ch->config.email.username) - 1);
    }
    if (password) {
        strncpy(ch->config.email.password, password, sizeof(ch->config.email.password) - 1);
    }
    ch->config.email.use_tls = use_tls;
    
    IDCU_LOG_INFO("notifier: email channel added, SMTP: %s:%d", smtp_host, smtp_port);
    return IDCU_ERR_SUCCESS;
}

int idcu_notifier_set_channel_enabled(idcu_Notifier* notifier, int index, int enabled)
{
    if (!notifier || !notifier->initialized || index < 0 || index >= notifier->channel_count) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    notifier->channels[index].enabled = enabled;
    IDCU_LOG_INFO("notifier: channel %d (%s) %s", 
                  index, 
                  idcu_notifier_channel_type_to_string(notifier->channels[index].type),
                  enabled ? "enabled" : "disabled");
    return IDCU_ERR_OK;
}

int idcu_notifier_remove_channel(idcu_Notifier* notifier, int index)
{
    if (!notifier || !notifier->initialized || index < 0 || index >= notifier->channel_count) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    for (int i = index; i < notifier->channel_count - 1; i++) {
        notifier->channels[i] = notifier->channels[i + 1];
    }
    notifier->channel_count--;
    
    IDCU_LOG_INFO("notifier: channel %d removed", index);
    return IDCU_ERR_OK;
}

int idcu_notifier_clear_channels(idcu_Notifier* notifier)
{
    if (!notifier || !notifier->initialized) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    memset(notifier->channels, 0, sizeof(notifier->channels));
    notifier->channel_count = 0;
    
    IDCU_LOG_INFO("notifier: all channels cleared");
    return IDCU_ERR_OK;
}

int idcu_notifier_send(idcu_Notifier* notifier, const idcu_AlertEvent* event)
{
    if (!notifier || !notifier->initialized || !event) {
        return IDCU_ERR_INVALID_PARAM;
    }
    
    int ret = IDCU_ERR_SUCCESS;
    
    for (int i = 0; i < notifier->channel_count; i++) {
        idcu_NotifierChannel* ch = &notifier->channels[i];
        if (!ch->enabled) {
            continue;
        }
        
        int channel_ret = IDCU_ERR_SUCCESS;
        switch (ch->type) {
            case IDCU_NOTIFIER_CHANNEL_LOG:
                channel_ret = send_to_log(event);
                break;
            case IDCU_NOTIFIER_CHANNEL_FILE:
                channel_ret = send_to_file(ch, event);
                break;
            case IDCU_NOTIFIER_CHANNEL_WEBHOOK:
                channel_ret = send_to_webhook(ch, event);
                break;
            case IDCU_NOTIFIER_CHANNEL_DINGTALK:
                channel_ret = send_to_dingtalk(ch, event);
                break;
            case IDCU_NOTIFIER_CHANNEL_WECHAT:
                channel_ret = send_to_wechat(ch, event);
                break;
            case IDCU_NOTIFIER_CHANNEL_EMAIL:
                channel_ret = send_to_email(ch, event);
                break;
            default:
                break;
        }
        
        if (channel_ret != IDCU_ERR_SUCCESS) {
            ret = channel_ret;
        }
    }
    
    return ret;
}

void idcu_notifier_alert_callback(const idcu_AlertEvent* event, void* user_data)
{
    idcu_Notifier* notifier = (idcu_Notifier*)user_data;
    if (notifier && notifier->initialized) {
        idcu_notifier_send(notifier, event);
    }
}
