# ESP8266-RTOS-SDK Blynk Components library

This open-source library ensures seamless integration between Blynk Cloud and devices on the ESP8288-RTOS-SDK. It offers
developers a free tool to optimize IoT communication, streamlining the development process for both commercial and
personal projects.

## Prerequisites

❯ CMake

❯ ESP8266-RTOS-SDK

## What is Blynk?

Blynk is a robust platform with iOS and Android apps that allow users to control any hardware, be it over the Internet
or directly through Bluetooth. This popular IoT solution, used worldwide by makers, design studios, educators, and
equipment vendors, simplifies the creation and management of IoT projects. The platform provides a digital dashboard
wherein users can craft a graphic interface for any IoT project by merely dragging and dropping widgets right on their
smartphone.

![blynk](https://user-images.githubusercontent.com/72824404/119525085-e464a300-bd86-11eb-84dc-a4f3de0f7ec9.png)

## How to use

Clone this repository somewhere, e.g.:

```shell
$ mkdir ~/workdir
$ cd ~/workdir
$ git clone https://github.com/CaCuCkA/esp8266-blynk-lib.git
$ cd esp8266-blynk-lib
```

> Replace `~/workdir` with the path to your workspace.

Add path to components in your project `CmakeLists.txt`, e.g:

```cmake
cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME example)
set(EXTRA_COMPONENT_DIRS
        /home/user/workdir/esp8266-blynk-lib/components)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(${PROJECT_NAME})
```

See [GitHub examples](https://github.com/CaCuCkA/esp8266-blynk-lib/tree/main/examples).

## Blynk Documentation

### Features Overview

#### - `blynk_err_t blynk_begin(blynk_device_t* device, const char* authentication_token)`

**Description**:

This function serves as the entry point in every Blynk program. It initializes the `blynk_device_t` structure with
default parameters:

- **Heartbeat Interval**:  
  This parameter determines how frequently the device checks its connection to the Blynk server. A common default value
  might be `2 seconds`, but this can be adjusted based on specific needs.

- **Reconnection Delay**:  
  In case the device loses its connection to the Blynk server, this parameter defines the waiting time before it tries
  to reconnect. A typical default value could be `5 seconds`.

- **Timeout**:  
  This is the maximum time the device waits for a response from the Blynk server before considering the request as
  failed. A common default might be `5 seconds`.

- **State Handler**:  
  By default, this function displays the Blynk device state such as: `AUTH`, `CONNECTED`, and `DISCONNECTED`.

---

#### - `blynk_state_t blynk_get_device_state(blynk_device_t* device)`

**Description**:

This function retrieves the current operational state of the specified Blynk device.

---

#### - Update default value functions

**Description**:

These functions allow you to modify the default settings of a Blynk device:

- `blynk_err_t update_default_timeout(blynk_device_t* device, tick_t timeout)`:
    - **Purpose**: Updates the default timeout duration for the device.

- `blynk_err_t update_heartbeat_interval(blynk_device_t* device, tick_t heartbeat_interval)`:
    - **Purpose**: Sets a new default heartbeat interval.

- `blynk_err_t update_default_reconnection_delay(blynk_device_t* device, tick_t reconnection_delay)`:
    - **Purpose**: Modifies the default reconnection delay.

- `blynk_err_t update_default_state_handler(blynk_device_t* device, blynk_state_handler_t handler, void* user_data)`:
    - **Purpose**: Updates the default state handler function.
    - **Details**: This function accepts a pointer to a function with the
      signature: `typedef void (* blynk_state_handler_t)(blynk_device_t*, const blynk_state_event_t*, void*)`.

---

#### - `blynk_err_t blynk_send_with_callback(blynk_device_t* device, uint8_t cmd, blynk_response_handler_t handler, void* data, tick_t wait, const char* fmt, ...)`

**Description**:

This function sends updates to the Blynk cloud for a specific pin. It can accept commands such as `BLYNK_CMD_HARDWARE`.

- The provided callback, denoted by the `blynk_response_callback_t` type, will be invoked upon a successful update. Its
  signature is: `typedef void (* blynk_response_callback_t)(blynk_device_t*, blynk_status_t, void*)`.
- Additionally, this function uses a format argument to determine the type of data being sent. More details on the
  format specification can be found in the  [Sending Value Format](#sending-value-format) section.

---

#### - `blynk_err_t blynk_send(blynk_device_t* device, blynk_cmd_t cmd, tick_t wait, const char* fmt, ...)`

**Description**:

This function facilitates communication with the Blynk cloud by sending specific commands.

- It accepts commands such as those denoted by the `blynk_cmd_t` type.
- The function uses a variable number of arguments, determined by the format string (`fmt`). This format argument
  specifies the type and order of data being sent.
- The `wait` parameter defines a timeout period for the send operation.
- For optimal operation and error handling, ensure the format string matches the provided data arguments.
-

---

#### - `blynk_err_t blynk_send_response(blynk_device_t* device, uint16_t id, uint16_t status, tick_t wait)`

**Description**:

This function is specifically designed to send response messages to the Blynk cloud.

- The `id` parameter represents the unique identifier associated with a particular Blynk request. It helps in
  correlating the response with the originating request.
- The `status` parameter is used to convey the success or failure status of a request or action.
- The `wait` parameter sets a timeout for the send operation, ensuring that the function doesn't hang indefinitely in
  case of network issues or other delays.
- Proper usage of this function helps in keeping the Blynk cloud updated about the status and results of various
  operations, enhancing the overall responsiveness of the system.

---

#### - `blynk_register_cmd_handler(blynk_device_t* device, const char* action, blynk_cmd_handler_t handler, void* data);`

**Description**:

This function registers a command handler that gets invoked when a specific command, to which it is bound, is detected
by the system.

- The `action` parameter specifies the command with which the handler will be associated.
- The `handler` conforms to the signature `typedef void (* blynk_cmd_handler_t)(blynk_handler_params_t* params)`. For
  more details about `blynk_handler_params_t`, refer to the [Data Structures](#data-structures) section.
- The `data` parameter allows for passing user-specific data into the handler.

---

#### - `blynk_err_t blynk_deregister_cmd_handler(blynk_device_t* device, const char* action)`

**Description**:

This function deregisters (removes) a previously registered command handler associated with the specified command. After
deregistration, the handler will no longer be invoked when the system detects the command.

- The `action` parameter specifies the command whose associated handler is to be deregistered.

--- 

#### - `blynk_err_t blynk_run(blynk_device_t* device)`

**Description**

This function serves as the primary loop of the library. **_After the initial setup and registration of all handlers_**,
this
function **_should be invoked_** to maintain the library's operations continuously.

### Exceptions codes

When your system crushed you will see such error log message:

> **<span style="color:red">ERROR</span>**
> `/home/user/path-to-your-project/components/blynk/src/internal/protocol_stuff.c:52`:  **[PROTOCOL STUFF]**: The
> function `disconnect_device` set the state to disconnected (reason: 4, code: 9).

To understand what error was occurred you should check exception enum where will be located `reason` and status code
enum
where will be located `code`:

* **_Reason_**

```c
typedef enum {
    BLYNK_EC_OK                 = 0,
    BLYNK_EC_MEM                = 1,
    BLYNK_EC_GAI                = 2,
    BLYNK_EC_ERRNO              = 3,
    BLYNK_EC_STATUS             = 4,
    BLYNK_EC_SYSTEM             = 5,
    BLYNK_EC_CLOSED             = 6,
    BLYNK_EC_TIMEOUT            = 7,
    BLYNK_EC_RUNNING            = 8,
    BLYNK_EC_NULL_PTR           = 9,
    BLYNK_EC_SET_OPTIONS        = 10,
    BLYNK_EC_NOT_CONNECTED      = 11,
    BLYNK_EC_FAILED_TO_READ     = 12,
    BLYNK_EC_INVALID_OPTION     = 13,
    BLYNK_EC_FAILED_TO_WRITE    = 14,
    BLYNK_EC_NOT_INITIALIZED    = 15,
    BLYNK_EC_NOT_AUTHENTICATED  = 16,
    BLYNK_EC_DEVICE_DISCONNECT  = 17,
} blynk_err_t;

```

* **_Status code_**

```c
typedef enum {
    BLYNK_STATUS_SUCCESS                    = 200,
    BLYNK_STATUS_QUOTA_LIMIT_EXCEPTION      = 1,
    BLYNK_STATUS_ILLEGAL_COMMAND            = 2,
    BLYNK_STATUS_NOT_REGISTERED             = 3,
    BLYNK_STATUS_ALREADY_REGISTERED         = 4,
    BLYNK_STATUS_NOT_AUTHENTICATED          = 5,
    BLYNK_STATUS_NOT_ALLOWED                = 6,
    BLYNK_STATUS_DEVICE_NOT_IN_NETWORK      = 7,
    BLYNK_STATUS_NO_ACTIVE_DASHBOARD        = 8,
    BLYNK_STATUS_INVALID_TOKEN              = 9,
    BLYNK_STATUS_ILLEGAL_COMMAND_BODY       = 11,
    BLYNK_STATUS_GET_GRAPH_DATA_EXCEPTION   = 12,
    BLYNK_STATUS_NO_DATA_EXCEPTION          = 17,
    BLYNK_STATUS_DEVICE_WENT_OFFLINE        = 18,
    BLYNK_STATUS_SERVER_EXCEPTION           = 19,

    BLYNK_STATUS_NTF_INVALID_BODY           = 13,
    BLYNK_STATUS_NTF_NOT_AUTHORIZED         = 14,
    BLYNK_STATUS_NTF_ECXEPTION              = 15,

    BLYNK_STATUS_TIMEOUT                    = 16,

    BLYNK_STATUS_NOT_SUPPORTED_VERSION      = 20,
    BLYNK_STATUS_ENERGY_LIMIT               = 21,

    BLYNK_STATUS_OPERATION_ACCEPT           = 23,
    BLYNK_STATUS_OPERATION_DECLINE          = 24,
} blynk_status_t;
```

### Data Structures

You will primarily interact with two Blynk data structures: `blynk_device_t` and` blynk_handler_params_t`. Here's an
explanation for the purpose of each field within these structures:

#### - `blynk_device_t`:

**Declaration:**

```c
struct blynk_device {
    bool valid;
    blynk_control_t control;
    blynk_private_data_t priv_data;
};
```

**Description:**

The `blynk_device_t` structure encapsulates details about the device's _connection state_, _authentication token_, and
_associated event handlers_. Users typically interact with this structure when managing Blynk device operations, but the
specific internal details are abstracted away for simplicity.

#### - `blynk_handler_params_t`

**Declaration:**

```c
struct blynk_handler_params {
    blynk_device_t* device;
    uint16_t id;
    const char* command;
    int argc;
    char** argv;
    void* data;
};
```

**Description**

This header should be passed in your custom command handler

- `device`: Refers to the associated Blynk device. It's essential for sending synchronized responses or messages back to
  the cloud from the handler.
- `id`: Represents the unique message identifier.
- `command`: Specifies which command in the header triggered this handler, e.g., vr, vw, etc.
- `argc`: Indicates the number of arguments sent from the cloud.
- `argv`: Contains the actual arguments sent from the cloud.
- `data`: Holds custom user-defined data.

### Sending Value Format

When using the synchronization functions such as `blynk_send`, the data you send can be formatted in various ways to
match the expected data types of the Blynk cloud. Below is a guide on how to format your data:

- **`c`**: Represents a single character (`char`).
- **`b`**: Represents a signed char integer (`signed char`).
- **`B`**: Represents an unsigned char (`unsigned char`).
- **`?`**: Represents a boolean value (`bool`).
- **`h`**: Represents a short integer (`short`).
- **`H`**: Represents an unsigned short integer (`unsigned short`).
- **`i`**: Represents an integer (`int`).
- **`I`**: Represents an unsigned integer (`unsigned int`).
- **`l`**: Represents a long integer (`long`).
- **`L`**: Represents an unsigned long integer (`unsigned long`).
- **`q`**: Represents a long long integer (`long long`).
- **`Q`**: Represents an unsigned long long integer (`unsigned long long`).
- **`f`**: Represents a floating-point number (`float`).
- **`d`**: Represents a double-precision floating-point number (`double`).
- **`s,p`**: Represents a string or a pointer to a character array (`char*`).

> ⚠️ **Note!**
>
> Your custom data format should include coommand that will be passed in BLynk message header
>
> Example:
> ```c
> blynk_send(params->device, BLYNK_CMD_HARDWARE, 100, "sii", "vw", pin, temperature);
>```
>

### Cross-Platform Compatibility

Adapting this library for another platform is straightforward. To do so, modify the functions
in [blynk_freertos_port.h](components%2Fblynk%2Finclude%2Fstuff%2Fblynk_freertos_port.h) and update the `FREERTOS`
macros in [CMakeLists.txt](components%2Fblynk%2FCMakeLists.txt) to match your platform.

If you decide to switch platforms, here's a sample function where you should integrate your implementation:

```c
semaphore_handle_t
create_semaphore(void) {
    semaphore_handle_t semaphore;

#ifdef FREERTOS
    semaphore = xSemaphoreCreateMutex();
#elif defined(USING_OTHEROS)
    // Replace with your non-FreeRTOS implementation
    semaphore = OtherOS_CreateMutex();
#else
#error "OS not supported!"
#endif

    return semaphore;
}
```

### FreeRTOS configurations

The Blynk library operates as a FreeRTOS task. Within [defines.h](components%2Fblynk%2Finclude%2Fstuff%2Fdefines.h)
there's a specified default stack size that you can
adjust to either increase or decrease based on your needs. Additionally, you have the flexibility to modify the queue
size, which dictates the number of simultaneous responses the system can handle.