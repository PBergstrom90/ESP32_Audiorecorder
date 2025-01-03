# ESP32 Audiorecorder

This project implements an audio recording system using an ESP32 microcontroller. It utilizes an I2S microphone for audio input, processes audio data, and transmits it securely via WebSocket to a server. The system supports both automatic and manual recording modes.

This project is built to connect with the Docker_Audiorecorder, which can be found in this repository: [Docker_Audiorecorder](https://github.com/PBergstrom90/Docker_Audiorecorder).

## Features

- **Audio Recording**:
  - Sampling rate: 16kHz
  - Mono audio
  - 32-bit samples
  - Adjustable gain
  - Noise threshold detection
  - 5-second recording duration (configurable)

- **Networking**:
  - Secure WebSocket communication over TLS (mTLS)
  - WiFi connectivity
  - Configurable server IP and port

- **System Modes**:
  - **Automatic Mode**: Listens for audio and triggers recording based on noise threshold.
  - **Manual Mode**: Recording triggered via Web or WebSocket command.

- **Web Interface**:
  - Start recording
  - Adjust gain
  - Toggle between automatic and manual modes

- **File System**:
  - Certificate management using LittleFS

## Hardware Requirements

- ESP32 microcontroller
- I2S microphone (e.g., INMP441)
- WiFi connectivity
- Server to receive WebSocket data

## Software Requirements

- Arduino IDE or PlatformIO
- ESPAsyncWebServer library
- WebSocketsClient library
- LittleFS library

## Configuration

### Pins

| Signal | GPIO Pin |
|--------|----------|
| WS     | 15       |
| SD     | 13       |
| SCK    | 2        |

### Audio Parameters

- Sampling Rate: 16kHz
- Bits per Sample: 32
- Gain: 0.3
- Noise Threshold: 4.0
- Recording Duration: 5000 ms

### WebSocket Configuration

- Server IP: `192.168.50.221`
- Port: `443`
- Reconnect Interval: 5000 ms
- Buffer Size: 1600 bytes
- Max Payload: 256 bytes

## System Components

### `config.h`
Defines constants for I2S, audio, and WebSocket configurations.

### `I2SMicrophone`
Handles microphone initialization, data reading, gain adjustment, state management, and error recovery.

### `MicrophoneTask`
Manages recording tasks, processes audio data, and triggers WebSocket communication.

### `SystemStateManager`
Tracks and toggles between automatic and manual modes.

### `WebServerHandler`
Provides an HTTP interface to control the microphone, set gain, and toggle system modes.

### `WebSocketHandler`
Manages secure WebSocket communication, handles TLS certificates, and sends audio data to the server.

## Setup

1. **Hardware Connections**:
   - Connect the I2S microphone to the ESP32 as per the pin configuration.

2. **File System**:
   - Place the certificates (`ca.crt`, `esp32.crt`, `esp32.key`) in the `/certs/` directory on LittleFS.

3. **Compile and Flash**:
   - Install required libraries.
   - Compile the project using PlatformIO or Arduino IDE.
   - Upload the firmware to the ESP32.

4. **Server Setup**:
   - Ensure a WebSocket server is running on the configured IP and port.
   - Validate server certificates.

5. **Run**:
   - Power on the ESP32.
   - Access the web interface or send WebSocket commands to control the system.

## Usage

### HTTP Endpoints

- `/start-record`: Start recording manually.
- `/set-gain?value=<gain>`: Adjust gain value.
- `/toggle-mode?mode=<automatic|manual>`: Switch system mode.

### WebSocket Messages

- `START`: Indicates the start of a recording.
- `END`: Indicates the end of a recording.
- `MODE:<automatic|manual>`: Indicates current mode.

## Troubleshooting

- **Missing Certificates**:
  - Ensure all required certificates are uploaded to LittleFS.

- **WiFi Connection Issues**:
  - Verify SSID and password.
  - Check for IP conflicts.

- **WebSocket Disconnection**:
  - Check server availability.
  - Verify TLS certificate validity.

## Future Improvements

- Implement "Wifi Light Sleep" functionality to save power.
- Implement dynamic configuration and more audiooptions through a web UI.
- Enhance error recovery mechanisms.

## License

[MIT License](LICENSE)