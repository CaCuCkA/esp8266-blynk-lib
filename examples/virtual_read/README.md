# Blynk IoT Virtual Read Example

This code exemplifies how to set up an IoT application using the Blynk platform to read commands and values sent to a
virtual pin. The primary focus is on initializing necessary configurations, establishing a WiFi connection, and
listening to Blynk commands sent to the virtual pin. Once a command is received, it will print out the command details,
pin, and pin state.

## Virtual read Command Handler
```c
static void
vr_handler(blynk_handler_params_t* params) {
    if (params->argc > 0) {
        char* pend;
        int pin = strtol(params->argv[0], &pend, 10);

        switch (pin) {
            case 1: {
                int temperature = get_temperature();
                blynk_send(params->device, BLYNK_CMD_HARDWARE, WAIT, "si", "vw", pin, temperature);
                break;
            }
            case 2: {
                int light_intensity = get_light_intensity();
                blynk_send(params->device, BLYNK_CMD_HARDWARE, WAIT, "si", "vw", pin, light_intensity);
                break;
            }
            default:
                printf("No handler for virtual pin V%d\n", pin);
                break;
        }
    } else {
        printf("VR command received with no arguments\n");
    }
}
```
The primary role of the vr_handler function is to interpret VR commands, determine which virtual pin's data is being
requested, fetch that data, and send it back to the Blynk server.

## Important Notes

* Always replace **_"YourAuthToken"_** with your unique Blynk authentication token.
* For added robustness, consider adding error handling, especially around the malloc function.
* Ensure you've set up your Blynk mobile application or any other Blynk client to send the virtual write command `vw`
  to communicate with this program.
* Consult the individual library documentation for more detailed info and other potential configurations.

