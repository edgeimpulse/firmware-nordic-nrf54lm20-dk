# Edge Impulse firmware for Nordic Semiconductor nRF54LM20 DK

[Edge Impulse](https://www.edgeimpulse.com) enables developers to create the next generation of intelligent device solutions with embedded Machine Learning. This repository contains the Edge Impulse firmware for the Nordic Semiconductor nRF54LM20 DK development board, in combination with the sensors. This combination supports all Edge Impulse device features, including ingestion and inferencing.

> **Note:** Do you just want to use this development board with Edge Impulse? No need to build this firmware. See the instructions for the [nRF54LM20DK](https://docs.edgeimpulse.com/docs/development-platforms/officially-supported-mcu-targets/nordic-semi-nrf54Llm20-dk) for prebuilt images and instructions, or use the [data forwarder](https://docs.edgeimpulse.com/docs/tools/edge-impulse-cli/cli-data-forwarder) to capture data from any sensor.

> **Note:** This firmware is compatible with NCS 3.4.1

## Hardware

The firmware reads motion data from an ST X-NUCLEO sensor expansion board. The board is **not** used as an Arduino/Zephyr shield: wire it directly to the DK's `i2c22` bus (SCL = P1.11, SDA = P1.12, with pull-ups enabled in the board overlay). Two sensor boards are supported:

| Sensor board       | Accelerometer (`0x19`) | Inertial / 6-axis IMU (`0x6b`) | Build option          |
| ------------------ | ---------------------- | ------------------------------ | --------------------- |
| X-NUCLEO-IKS02A1   | IIS2DLPC               | ISM330DHCX                     | default               |
| X-NUCLEO-IKS01A3   | LIS2DW12               | LSM6DSO                        | `-DUSE_IKS01A3=1`     |

The selection is made in `CMakeLists.txt` and picks both the devicetree overlay (`boards/iks02a1.overlay` or `boards/iks01a3.overlay`) and the matching sources (`src/sensors/iks02a1/` or `src/sensors/iks01a3/`). The two must stay in sync, so always select the sensor board via `USE_IKS01A3` rather than editing a single file.

## Board revisions

The DK comes in two silicon revisions, and the board name must include the revision. The old name `nrf54lm20dk/nrf54lm20/cpuapp` no longer works.

| Revision | West board name                  | Board overlay                                 |
| -------- | -------------------------------- | --------------------------------------------- |
| A        | `nrf54lm20dk/nrf54lm20a/cpuapp`  | `boards/nrf54lm20dk_nrf54lm20a_cpuapp.overlay` |
| B        | `nrf54lm20dk/nrf54lm20b/cpuapp`  | `boards/nrf54lm20dk_nrf54lm20b_cpuapp.overlay` |

## Building the device firmware (locally)

1. Install the [nRF Connect SDK](https://nrfconnectdocs.nordicsemi.com/ncs/3.4.1/nrf/installation/install_ncs.html) in a *separate* folder from this repository (e.g. `~/repos/ncs`).

2. Clone this repository:

    ```bash
    $ git clone https://github.com/edgeimpulse/firmware-nordic-nrf54lm20-dk
    ```

3. Build the application (replace `nrf54lm20b` with `nrf54lm20a` for revision A boards):

    ```bash
    # X-NUCLEO-IKS02A1 (default)
    $ west build -p -b nrf54lm20dk/nrf54lm20b/cpuapp

    # X-NUCLEO-IKS01A3
    $ west build -p -b nrf54lm20dk/nrf54lm20b/cpuapp -- -DUSE_IKS01A3=1
    ```

    Use `-p` (pristine) when switching sensor boards, as the devicetree overlay choice is cached in the build directory.

## Building the device firmware (Docker)

1. Clone this repository:

    ```bash
    $ git clone https://github.com/edgeimpulse/firmware-nordic-nrf54lm20-dk
    ```

2. Build the Docker container:

    ```bash
    $ docker build -t edge-impulse-nordic-axon .
    ```

3. Build the application:

    ```bash
    # X-NUCLEO-IKS02A1 (default)
    $ docker run --rm -v $PWD:/app edge-impulse-nordic-axon west build -p -b nrf54lm20dk/nrf54lm20b/cpuapp

    # X-NUCLEO-IKS01A3
    $ docker run --rm -v $PWD:/app edge-impulse-nordic-axon west build -p -b nrf54lm20dk/nrf54lm20b/cpuapp -- -DUSE_IKS01A3=1
    ```

## Flashing

1. Connect the board and power it on.
2. Copy `build/zephyr/zephyr.bin` to the `JLINK` mass storage device.

## Flashing (command line)

1. Connect the board and power on.
2. Flash the board controller firmware:

    ```bash
    $ west flash
    ```
