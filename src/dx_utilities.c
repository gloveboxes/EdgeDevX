#include "dx_utilities.h"

static char *_log_debug_buffer = NULL;
static size_t _log_debug_buffer_size;
static volatile bool network_timer_initialised = false;
static bool network_connected_cached = false;

static const char end_to_end_test_url[] = "http://www.msftconnecttest.com";

static DX_DECLARE_TIMER_HANDLER(network_connected_expired_handler);
static DX_TIMER_BINDING tmr_network_connected_cached = {.name = "tmr_network_connected_cached", .handler = network_connected_expired_handler};

// Curl stuff.
struct url_data
{
    size_t size;
    char *data;
};

enum
{
    IFACE_IPv4 = (1 << 0),
    IFACE_IPv6 = (1 << 1),
    IFACE_IP = (1 << 0) | (1 << 1),
};

bool dx_isStringNullOrEmpty(const char *string)
{
    return string == NULL || strlen(string) == 0;
}

/// <summary>
/// check string contain only printable characters
/// ! " # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7 8 9 : ; < = > ? @ A B C D E F G H I J K L M N O P Q
/// R S T U V W X Y Z [ \ ] ^ _ ` a b c d e f g h i j k l m n o p q r s t u v w x y z { | } ~
/// </summary>
/// <param name="data"></param>
/// <returns></returns>
bool dx_isStringPrintable(char *data)
{
    while (isprint(*data))
    {
        data++;
    }
    return 0x00 == *data;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, struct url_data *data)
{
    size_t index = data->size;
    size_t n = (size * nmemb);
    char *tmp;

    data->size += (size * nmemb);

    tmp = (char *)realloc(data->data, data->size + 1); /* +1 for '\0' */

    if (tmp)
    {
        data->data = tmp;
    }
    else
    {
        if (data->data)
        {
            free(data->data);
        }
        return 0;
    }

    memcpy((data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return size * nmemb;
}

char *dx_getHttpData(const char *url)
{
    struct url_data data;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    data.size = 0;
    data.data = malloc(1);

    CURLcode res = CURLE_OK;

    CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    /* use a GET to fetch data */
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

    // based on the libcurl sample - https://curl.se/libcurl/c/https.html
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    /* Perform the request */
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res == CURLE_OK)
    {
        // caller is responsible for freeing this.
        return data.data;
    }
    else
    {
        free(data.data);
    }

    return NULL;
}

bool dx_isNetworkReady(void)
{
    struct ifaddrs *curr, *list = NULL;
    int result = 0;

    if (getifaddrs(&list) == -1)
        return 0;

    for (curr = list; curr != NULL; curr = curr->ifa_next)
    {
        if (strcmp("lo", curr->ifa_name) != 0)
        {
            if (curr->ifa_addr->sa_family == AF_INET)
                result |= IFACE_IPv4;
            if (curr->ifa_addr->sa_family == AF_INET6)
                result |= IFACE_IPv6;
        }
    }

    freeifaddrs(list);
    errno = 0;
    
    return result;
}

DX_TIMER_HANDLER(network_connected_expired_handler)
{
    network_connected_cached = false;
}
DX_TIMER_HANDLER_END

bool dx_isNetworkConnected(const char *networkInterface)
{
    bool network_ready = false;
    char *network_connected_result = NULL;

    if (!network_timer_initialised)
    {
        network_timer_initialised = true;
        dx_timerStart(&tmr_network_connected_cached);
    }

    if (!(network_ready = dx_isNetworkReady())){
        return network_ready;
    }

    // no network interface provided for end to end test so we'll go with the result of dx_isNetworkReady result
    if (dx_isStringNullOrEmpty(networkInterface)){
        return network_ready;
    }

    if (network_connected_cached) {
        return network_ready;
    }

    network_connected_result = dx_getHttpData(end_to_end_test_url);

    if (network_connected_result == NULL) {
        return false;
    } else {
        free(network_connected_result);
        network_connected_result = NULL;

        network_connected_cached = true;
        // 3 minute cache on end to end testing
        dx_timerOneShotSet(&tmr_network_connected_cached, &(struct timespec){3 * 60, 0});
    }

    return true;
}

bool dx_isDeviceAuthReady(void)
{
    // // Verifies authentication is ready on device
    // bool currentAppDeviceAuthReady = false;
    // if (Application_IsDeviceAuthReady(&currentAppDeviceAuthReady) != 0) {
    //     Log_Debug("ERROR: Application_IsDeviceAuthReady: %d (%s)\n", errno, strerror(errno));
    // } else {
    //     if (!currentAppDeviceAuthReady) {
    //         Log_Debug("ERROR: Current Application not Device Auth Ready\n");
    //     }
    // }

    // return currentAppDeviceAuthReady;

    return true;
}

char *dx_getCurrentUtc(char *buffer, size_t bufferSize)
{
    time_t now = time(NULL);
    struct tm *t = gmtime(&now);
    strftime(buffer, bufferSize - 1, "%Y-%m-%dT%H:%M:%SZ", t);
    return buffer;
}

int64_t dx_getNowMilliseconds(void)
{
    struct timespec now = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

int dx_stringEndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix > lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

bool dx_startThreadDetached(void *(daemon)(void *), void *arg, char *daemon_name)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_t thread;

    printf("Starting thread %s detached\n", daemon_name);

    if (pthread_create(&thread, &attr, daemon, arg))
    {
        printf("ERROR: Failed to start %s daemon.\n", daemon_name);
        return false;
    }
    return true;
}

void dx_Log_Debug_Init(char *buffer, size_t buffer_size)
{
    _log_debug_buffer = (char *)buffer;
    _log_debug_buffer_size = buffer_size;
}

void dx_Log_Debug(char *fmt, ...)
{
    if (_log_debug_buffer == NULL)
    {
        printf("log_debug_buffer is NULL. Call Log_Debug_Time_Init first");
        return;
    }

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("%02d:%02d:%02d - ", tm.tm_hour, tm.tm_min, tm.tm_sec);

    va_list args;
    va_start(args, fmt);
    vsnprintf(_log_debug_buffer, _log_debug_buffer_size, fmt, args);
    va_end(args);

    printf("%s", _log_debug_buffer);
}