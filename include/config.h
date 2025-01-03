#ifndef CONFIG_H
#define CONFIG_H

// I2S Microphone Configuration
#define I2S_WS          15      // Word Select Pin
#define I2S_SD          13      // Serial Data Pin
#define I2S_SCK         2       // Serial Clock Pin
#define I2S_PORT        I2S_NUM_0 // I2S Port Number
#define WARM_UP_PASSES  10      // Number of warm-up passes

// Audio Configuration
#define SAMPLE_RATE     16000   // Sampling Rate in Hz
#define CHANNEL_COUNT   1       // Mono Audio
#define BITS_PER_SAMPLE 32      // Bits per sample
#define RECORD_DURATION_MS 5000 // Recording Duration (5 seconds)
#define GAIN_VALUE      0.3     // Default gain value
#define NOISE_THRESHOLD 4.0     // Noise threshold

// WebSocket Configuration
#define SOCKET_PORT_NUMBER 443
#define RECONNECT_INTERVAL 5000 // Milliseconds
#define HOST_SOCKET_IP "192.168.50.221" // IP for the server
#define WEBSOCKET_BUFFER_SIZE 1600
#define MAX_PAYLOAD_SIZE 256

#endif