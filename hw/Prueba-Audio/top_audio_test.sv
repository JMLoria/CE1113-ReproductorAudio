module top_audio_test (
    input  logic        CLOCK_50,
    input  logic        KEY0,         // Reset activo-bajo

    // Audio CODEC — pines físicos DE1-SoC
    input  logic        AUD_ADCDAT,
    input  logic         AUD_BCLK,
    input  logic         AUD_ADCLRCK,
    input  logic         AUD_DACLRCK,
    output logic        AUD_DACDAT,
    output logic        AUD_XCK,

    // I2C para configurar el WM8731
    output logic        FPGA_I2C_SCLK,
	 output logic [9:0] LEDR,
    inout  wire         FPGA_I2C_SDAT
);

    wire pll_locked;
    wire dacdat_nc;   // DACDAT del soc_system no se usa en bypass

    // ── 1. PLL: 50 MHz → 12.288 MHz para AUD_XCK ─────────────
    pll_audio pll_inst (
        .refclk   (CLOCK_50),
        .rst      (~KEY0),     // PLL usa reset activo-alto
        .outclk_0 (AUD_XCK),
        .locked   (pll_locked)
    );

    // ── 2. soc_system: configura el WM8731 via I2C ────────────
    soc_system u0 (
        .clk_clk                        (CLOCK_50),
        .reset_reset_n                  (KEY0),
        .external_interface_SDAT        (FPGA_I2C_SDAT),
        .external_interface_SCLK        (FPGA_I2C_SCLK),
        .external_interface_1_ADCDAT    (AUD_ADCDAT),
        .external_interface_1_ADCLRCK   (AUD_ADCLRCK),
        .external_interface_1_BCLK      (AUD_BCLK),
        .external_interface_1_DACDAT    (dacdat_nc),
        .external_interface_1_DACLRCK   (AUD_DACLRCK)
    );

    ToneGenerator tone_gen (
        .AUD_BCLK    (AUD_BCLK),
        .AUD_DACLRCK (AUD_DACLRCK),
        .AUD_DACDAT  (AUD_DACDAT)
    );
	 
	 assign LEDR[0] = pll_locked;     // LED0 encendido = PLL estable
	 assign LEDR[9:2] = '0;
	 
	 logic [15:0] clk_div;
	 always_ff @(posedge AUD_DACLRCK)
		  clk_div <= clk_div + 16'd1;

	 assign LEDR[1] = clk_div[15];  // parpadea si DACLRCK está activo

endmodule