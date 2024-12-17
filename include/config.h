#ifndef CONFIG_H
#define CONFIG_H

// I2S Microphone Configuration
#define I2S_WS          15      // Word Select Pin
#define I2S_SD          13      // Serial Data Pin
#define I2S_SCK         2       // Serial Clock Pin
#define I2S_PORT        I2S_NUM_0

// Audio Configuration
#define SAMPLE_RATE     16000   // Sampling Rate in Hz
#define CHANNEL_COUNT   1       // Mono Audio
#define BITS_PER_SAMPLE 16      // Bits per sample
#define RECORD_DURATION_MS 5000 // Recording Duration (5 seconds)

#endif