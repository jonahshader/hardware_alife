#include <gtest/gtest.h>
#include <memory>
#include "Vrw_fb_test_top.h"
#include "verilated.h"

class RwFbTest : public ::testing::Test {
protected:
    std::unique_ptr<Vrw_fb_test_top> dut;
    uint64_t time_counter;

    void SetUp() override {
        dut = std::make_unique<Vrw_fb_test_top>();
        time_counter = 0;

        // Initialize inputs
        dut->disp_clk = 0;
        dut->gpu_clk = 0;
        dut->swap = 0;
        dut->disp_read_pos_x = 0;
        dut->disp_read_pos_y = 0;
        dut->gpu_read_pos_x = 0;
        dut->gpu_read_pos_y = 0;
        dut->gpu_write_pos_x = 0;
        dut->gpu_write_pos_y = 0;
        dut->gpu_write_val_r = 0;
        dut->gpu_write_val_g = 0;
        dut->gpu_write_val_b = 0;
        dut->gpu_write = 0;

        // Initial evaluation
        dut->eval();
    }

    void TearDown() override {
        dut->final();
    }

    // Helper to advance time and complete full clock cycles
    void tick_clocks() {
        time_counter++;

        // Falling edge for both clocks
        dut->gpu_clk = 0;
        dut->disp_clk = 0;
        dut->eval();

        // Rising edge for both clocks
        dut->gpu_clk = 1;
        dut->disp_clk = 1;
        dut->eval();
    }

    // Helper to set framebuffer position
    void set_fb_pos(uint8_t &x_port, uint8_t &y_port, uint8_t x, uint8_t y) {
        x_port = x;
        y_port = y;
    }

    // Helper to set RGB value
    void set_rgb(uint8_t &r_port, uint8_t &g_port, uint8_t &b_port, uint8_t r, uint8_t g, uint8_t b) {
        r_port = r & 0xF;
        g_port = g & 0xF;
        b_port = b & 0xF;
    }

    // Helper to get RGB value as packed uint16_t for comparison
    uint16_t get_rgb(uint8_t r, uint8_t g, uint8_t b) {
        return ((r & 0xF) << 8) | ((g & 0xF) << 4) | (b & 0xF);
    }
};

TEST_F(RwFbTest, BasicInstantiation) {
    EXPECT_NE(dut, nullptr);
}

TEST_F(RwFbTest, WriteAndReadPixel) {
    // Test writing a pixel and reading it back
    uint8_t test_x = 10, test_y = 20;
    uint8_t test_r = 0xF, test_g = 0x8, test_b = 0x4;

    // Set up write operation
    set_fb_pos(dut->gpu_write_pos_x, dut->gpu_write_pos_y, test_x, test_y);
    set_rgb(dut->gpu_write_val_r, dut->gpu_write_val_g, dut->gpu_write_val_b, test_r, test_g, test_b);
    dut->gpu_write = 1;

    // Perform write with clock edge
    tick_clocks();

    // Disable write
    dut->gpu_write = 0;

    // Set up read operation at same position
    set_fb_pos(dut->gpu_read_pos_x, dut->gpu_read_pos_y, test_x, test_y);

    // Allow for read latency (module has READ_LATENCY parameter)
    // We'll tick several times to account for the delay
    for (int i = 0; i < 5; i++) {
        tick_clocks();
    }

    // The read value should match what we wrote
    uint16_t expected = get_rgb(test_r, test_g, test_b);
    uint16_t actual = get_rgb(dut->gpu_read_val_r, dut->gpu_read_val_g, dut->gpu_read_val_b);
    EXPECT_EQ(actual, expected);
}

TEST_F(RwFbTest, BufferSwapping) {
    // Test that buffer swapping works correctly
    uint8_t test_x = 5, test_y = 5;
    uint8_t r1 = 0xF, g1 = 0x0, b1 = 0x0; // Red
    uint8_t r2 = 0x0, g2 = 0xF, b2 = 0x0; // Green

    // Write to current buffer
    set_fb_pos(dut->gpu_write_pos_x, dut->gpu_write_pos_y, test_x, test_y);
    set_rgb(dut->gpu_write_val_r, dut->gpu_write_val_g, dut->gpu_write_val_b, r1, g1, b1);
    dut->gpu_write = 1;
    tick_clocks();
    dut->gpu_write = 0;

    // Perform buffer swap
    dut->swap = 1;
    tick_clocks();
    dut->swap = 0;

    // Write different color to new buffer
    set_fb_pos(dut->gpu_write_pos_x, dut->gpu_write_pos_y, test_x, test_y);
    set_rgb(dut->gpu_write_val_r, dut->gpu_write_val_g, dut->gpu_write_val_b, r2, g2, b2);
    dut->gpu_write = 1;
    tick_clocks();
    dut->gpu_write = 0;

    // Set up read
    set_fb_pos(dut->gpu_read_pos_x, dut->gpu_read_pos_y, test_x, test_y);

    // Wait for read latency
    for (int i = 0; i < 5; i++) {
        tick_clocks();
    }

    // Should read the new color (color2) since we're reading from current buffer
    uint16_t expected = get_rgb(r2, g2, b2);
    uint16_t actual = get_rgb(dut->gpu_read_val_r, dut->gpu_read_val_g, dut->gpu_read_val_b);
    EXPECT_EQ(actual, expected);
}

TEST_F(RwFbTest, DisplayRead) {
    // Test display reading functionality
    uint8_t test_x = 15, test_y = 25;
    uint8_t test_r = 0x0, test_g = 0x0, test_b = 0xF; // Blue

    // Write to GPU buffer
    set_fb_pos(dut->gpu_write_pos_x, dut->gpu_write_pos_y, test_x, test_y);
    set_rgb(dut->gpu_write_val_r, dut->gpu_write_val_g, dut->gpu_write_val_b, test_r, test_g, test_b);
    dut->gpu_write = 1;
    tick_clocks();
    dut->gpu_write = 0;

    // Set display read position
    set_fb_pos(dut->disp_read_pos_x, dut->disp_read_pos_y, test_x, test_y);

    // Wait for read latency
    for (int i = 0; i < 5; i++) {
        tick_clocks();
    }

    // Display should read from the opposite buffer initially (before swap)
    // So it might read default value initially

    // Perform swap to make written data visible to display
    dut->swap = 1;
    tick_clocks();
    dut->swap = 0;

    // Wait for cross-domain synchronization and read latency
    for (int i = 0; i < 10; i++) {
        tick_clocks();
    }

    // Now display should see the written value
    uint16_t expected = get_rgb(test_r, test_g, test_b);
    uint16_t actual = get_rgb(dut->disp_read_val_r, dut->disp_read_val_g, dut->disp_read_val_b);
    EXPECT_EQ(actual, expected);
}

TEST_F(RwFbTest, MultiplePixelWrites) {
    // Test writing and reading multiple pixels
    struct PixelData {
        uint8_t x, y;
        uint8_t r, g, b;
    };

    std::vector<PixelData> pixels = {
        {0, 0, 0xF, 0x0, 0x0},    // Red at (0,0)
        {1, 0, 0x0, 0xF, 0x0},    // Green at (1,0)
        {0, 1, 0x0, 0x0, 0xF},    // Blue at (0,1)
        {1, 1, 0xF, 0xF, 0xF},     // White at (1,1)
        {0, 2, 0xA, 0xB, 0xC},
        {2, 1, 0x1, 0x3, 0X5}
    };

    // Write all pixels
    for (const auto& pixel : pixels) {
        set_fb_pos(dut->gpu_write_pos_x, dut->gpu_write_pos_y, pixel.x, pixel.y);
        set_rgb(dut->gpu_write_val_r, dut->gpu_write_val_g, dut->gpu_write_val_b, pixel.r, pixel.g, pixel.b);
        dut->gpu_write = 1;
        tick_clocks();
        dut->gpu_write = 0;
    }

    // Read back all pixels and verify
    for (const auto& pixel : pixels) {
        set_fb_pos(dut->gpu_read_pos_x, dut->gpu_read_pos_y, pixel.x, pixel.y);

        // Wait for read latency
        for (int i = 0; i < 5; i++) {
            tick_clocks();
        }

        uint16_t expected = get_rgb(pixel.r, pixel.g, pixel.b);
        uint16_t actual = get_rgb(dut->gpu_read_val_r, dut->gpu_read_val_g, dut->gpu_read_val_b);
        EXPECT_EQ(actual, expected)
            << "Pixel mismatch at (" << (int)pixel.x << "," << (int)pixel.y << ")";
    }
}
