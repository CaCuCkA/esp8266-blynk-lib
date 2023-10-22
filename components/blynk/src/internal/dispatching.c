#include "stuff/util.h"
#include "stuff/defines.h"
#include "stuff/exceptions.h"
#include "internal/dispatching.h"
#include "internal/internal_comm.h"


static blynk_state_t blynk_get_state(blynk_device_t* device);

static blynk_err_t check_device_state(blynk_device_t* device, blynk_cmd_t cmd);

static void construct_payload(char* payload, uint16_t* len, const char* fmt, va_list ap);

static void initialize_package(blynk_packet_t* package, blynk_device_t* device, blynk_cmd_t cmd,
                               uint16_t len, char* payload, blynk_response_handler_t handler,
                               void* data, TickType_t wait);


blynk_err_t dispatch_blynk_request(blynk_device_t* device, blynk_cmd_t cmd,
                                   blynk_response_handler_t handler, void* data,
                                   TickType_t wait, const char* fmt, va_list ap) {

    blynk_err_t err = check_device_state(device, cmd);
    if (err != BLYNK_EC_OK) {
        return err;
    }

    char payload[BLYNK_MAX_PAYLOAD_LEN];
    uint16_t len = 0;
    construct_payload(payload, &len, fmt, ap);

    blynk_packet_t package;
    initialize_package(&package, device, cmd, len, payload, handler, data, wait);

    return blynk_notify_packet_ready(&package);
}


static blynk_err_t
check_device_state(blynk_device_t* device, blynk_cmd_t cmd) {
    if (!BLYNK_DEVICE_IS_VALID(device)) {
        return BLYNK_EC_NOT_INITIALIZED;
    }

    if (cmd == BLYNK_CMD_RESPONSE) {
        return BLYNK_EC_INVALID_OPTION;
    }

    blynk_state_t state = blynk_get_state(device);
    if (state == BLYNK_STATE_STOPPED || state == BLYNK_STATE_DISCONNECTED) {
        return BLYNK_EC_NOT_CONNECTED;
    } else if (state == BLYNK_STATE_CONNECTED) {
        return BLYNK_EC_NOT_AUTHENTICATED;
    }

    return BLYNK_EC_OK;
}


static void
construct_payload(char* payload, uint16_t* len, const char* fmt, va_list ap) {
    char* payload_ptr = payload;

    for (; *fmt && *len < BLYNK_MAX_PAYLOAD_LEN; fmt++) {
        char format_buffer[FORMAT_BUFFER_SIZE];
        const char* arg;

        if (*fmt == 's' || *fmt == 'p') {
            arg = va_arg(ap, const char*);
        } else {
            switch (*fmt) {
                case 'c':
                case 'b':
                            FALLTHROUGH;
                case 'B':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%c", va_arg(ap, int));
                    break;

                case '?':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%s", va_arg(ap, bool) ? "true" : "false");
                    break;

                case 'h':
                case 'H':
                            FALLTHROUGH;
                case 'i':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%d", va_arg(ap, int));
                    break;

                case 'I':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%u", va_arg(ap, unsigned int));
                    break;

                case 'l':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%ld", va_arg(ap, long));
                    break;

                case 'L':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%lu", va_arg(ap, unsigned long));
                    break;

                case 'q':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%lld", va_arg(ap, long long));
                    break;

                case 'Q':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%llu", va_arg(ap, unsigned long long));
                    break;

                case 'f':
                            FALLTHROUGH;
                case 'd':
                    snprintf(format_buffer, FORMAT_BUFFER_SIZE, "%.7f", va_arg(ap, double));
                    break;

                default:
                    continue;
            }
            arg = format_buffer;
        }

        while (*arg && *len < BLYNK_MAX_PAYLOAD_LEN) {
            *(payload_ptr++) = *(arg++);
            (*len)++;
        }

        if (*len < BLYNK_MAX_PAYLOAD_LEN && *(fmt + 1)) {
            *(payload_ptr++) = 0;
            (*len)++;
        }
    }
}


static blynk_state_t
blynk_get_state(blynk_device_t* device) {
    mutex_wrap_t wrap = {
            .type = MUTEX_TYPE_FREERTOS,
            .mutex = {device->control.mtx},
    };

    mutex_wrapper_take(&wrap);
    blynk_state_t state = device->control.state;
    mutex_wrapper_give(&wrap);

    return state;
}


static void
initialize_package(blynk_packet_t* package, blynk_device_t* device, blynk_cmd_t cmd,
                   uint16_t len, char* payload, blynk_response_handler_t handler,
                   void* data, TickType_t wait) {
    package->device = device;
    package->cmd = cmd;
    package->id = 0;
    package->len = len;
    package->payload = len > 0 ? (uint8_t*) payload : NULL;
    package->handler = handler;
    package->data = data;
    package->wait = wait;
}
