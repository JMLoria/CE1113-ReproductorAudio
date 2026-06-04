module top_audio_test (
    input  logic        CLOCK_50,
    input  logic [3:0]  KEY,
    input  logic [9:0]  SW,

    // Audio CODEC
    input  logic        AUD_ADCDAT,
    input  logic        AUD_BCLK,
    input  logic        AUD_ADCLRCK,
    input  logic        AUD_DACLRCK,
    output logic        AUD_DACDAT,
    output logic        AUD_XCK,

    // I2C para configurar el WM8731
    output logic        FPGA_I2C_SCLK,
    inout  wire         FPGA_I2C_SDAT,

    // LEDs y 7 segmentos
    output logic [9:0]  LEDR,
    output logic [6:0]  HEX0,
    output logic [6:0]  HEX1,
    output logic [6:0]  HEX2,
    output logic [6:0]  HEX3,
    output logic [6:0]  HEX4,
    output logic [6:0]  HEX5,

    // HPS DDR3
    output wire [14:0] HPS_DDR3_ADDR,
    output wire [2:0]  HPS_DDR3_BA,
    output wire        HPS_DDR3_CK_P,
    output wire        HPS_DDR3_CK_N,
    output wire        HPS_DDR3_CKE,
    output wire        HPS_DDR3_CS_N,
    output wire        HPS_DDR3_RAS_N,
    output wire        HPS_DDR3_CAS_N,
    output wire        HPS_DDR3_WE_N,
    output wire        HPS_DDR3_RESET_N,
    inout  wire [31:0] HPS_DDR3_DQ,
    inout  wire [3:0]  HPS_DDR3_DQS_P,
    inout  wire [3:0]  HPS_DDR3_DQS_N,
    output wire        HPS_DDR3_ODT,
    output wire [3:0]  HPS_DDR3_DM,
    input  wire        HPS_DDR3_RZQ,

    // HPS Ethernet
    output wire        HPS_ENET_GTX_CLK,
    output wire [3:0]  HPS_ENET_TX_DATA,
    output wire        HPS_ENET_TX_EN,
    input  wire        HPS_ENET_RX_CLK,
    input  wire [3:0]  HPS_ENET_RX_DATA,
    input  wire        HPS_ENET_RX_DV,
    output wire        HPS_ENET_MDC,
    inout  wire        HPS_ENET_MDIO,

    // HPS SD
    output wire        HPS_SD_CLK,
    inout  wire        HPS_SD_CMD,
    inout  wire [3:0]  HPS_SD_DATA,

    // HPS SPI
    output wire        HPS_SPIM_CLK,
    output wire        HPS_SPIM_MOSI,
    input  wire        HPS_SPIM_MISO,
    output wire        HPS_SPIM_SS,

    // HPS UART
    input  wire        HPS_UART_RX,
    output wire        HPS_UART_TX,

    // HPS I2C
    inout  wire        HPS_I2C1_SDAT,
    inout  wire        HPS_I2C1_SCLK,
    inout  wire        HPS_I2C2_SDAT,
    inout  wire        HPS_I2C2_SCLK,

    // HPS GPIO
    inout  wire [1:0]  HPS_GPIO
);

    wire dacdat_nc;

    // Sale del registro Avalon-MM escrito por Nios
    logic [1:0] filter_sel_from_nios;

    // LEDs escritos desde Nios por PIO, pero no los usamos directo en esta prueba
    wire [9:0] leds_from_nios;

    // --------------------------------------------------------------------
    // soc_system: HPS + Nios + PIO switches + audio config + filter control
    // --------------------------------------------------------------------
    soc_system u0 (
        .clk_clk                         (CLOCK_50),

        // Audio IP / codec
        .audio_BCLK                      (AUD_BCLK),
        .audio_DACLRCK                   (AUD_DACLRCK),
        .audio_DACDAT                    (dacdat_nc),

        // Clock de audio generado por el audio_pll dentro de soc_system
        .audio_clk_clk                   (AUD_XCK),

        // I2C para configurar WM8731
        .audio_config_SDAT               (FPGA_I2C_SDAT),
        .audio_config_SCLK               (FPGA_I2C_SCLK),

        // Botones y switches
        .buttons_export                  (KEY),
        .switches_export                 (SW),

        // Registro Avalon-MM de seleccion de filtro
        .filter_sel_from_nios_export     (filter_sel_from_nios),

        // HEX desde el controlador de 7 segmentos
        .hex_hex0                        (HEX0),
        .hex_hex1                        (HEX1),
        .hex_hex2                        (HEX2),
        .hex_hex3                        (HEX3),
        .hex_hex4                        (HEX4),
        .hex_hex5                        (HEX5),

        // PIO LEDs de Nios
        .leds_export                     (leds_from_nios),

        // HPS DDR3
        .memory_mem_a                    (HPS_DDR3_ADDR),
        .memory_mem_ba                   (HPS_DDR3_BA),
        .memory_mem_ck                   (HPS_DDR3_CK_P),
        .memory_mem_ck_n                 (HPS_DDR3_CK_N),
        .memory_mem_cke                  (HPS_DDR3_CKE),
        .memory_mem_cs_n                 (HPS_DDR3_CS_N),
        .memory_mem_ras_n                (HPS_DDR3_RAS_N),
        .memory_mem_cas_n                (HPS_DDR3_CAS_N),
        .memory_mem_we_n                 (HPS_DDR3_WE_N),
        .memory_mem_reset_n              (HPS_DDR3_RESET_N),
        .memory_mem_dq                   (HPS_DDR3_DQ),
        .memory_mem_dqs                  (HPS_DDR3_DQS_P),
        .memory_mem_dqs_n                (HPS_DDR3_DQS_N),
        .memory_mem_odt                  (HPS_DDR3_ODT),
        .memory_mem_dm                   (HPS_DDR3_DM),
        .memory_oct_rzqin                (HPS_DDR3_RZQ),

        // HPS Ethernet
        .hps_io_hps_io_emac1_inst_TX_CLK (HPS_ENET_GTX_CLK),
        .hps_io_hps_io_emac1_inst_TXD0   (HPS_ENET_TX_DATA[0]),
        .hps_io_hps_io_emac1_inst_TXD1   (HPS_ENET_TX_DATA[1]),
        .hps_io_hps_io_emac1_inst_TXD2   (HPS_ENET_TX_DATA[2]),
        .hps_io_hps_io_emac1_inst_TXD3   (HPS_ENET_TX_DATA[3]),
        .hps_io_hps_io_emac1_inst_TX_CTL (HPS_ENET_TX_EN),
        .hps_io_hps_io_emac1_inst_RX_CLK (HPS_ENET_RX_CLK),
        .hps_io_hps_io_emac1_inst_RXD0   (HPS_ENET_RX_DATA[0]),
        .hps_io_hps_io_emac1_inst_RXD1   (HPS_ENET_RX_DATA[1]),
        .hps_io_hps_io_emac1_inst_RXD2   (HPS_ENET_RX_DATA[2]),
        .hps_io_hps_io_emac1_inst_RXD3   (HPS_ENET_RX_DATA[3]),
        .hps_io_hps_io_emac1_inst_RX_CTL (HPS_ENET_RX_DV),
        .hps_io_hps_io_emac1_inst_MDC    (HPS_ENET_MDC),
        .hps_io_hps_io_emac1_inst_MDIO   (HPS_ENET_MDIO),

        // HPS SD
        .hps_io_hps_io_sdio_inst_CMD     (HPS_SD_CMD),
        .hps_io_hps_io_sdio_inst_D0      (HPS_SD_DATA[0]),
        .hps_io_hps_io_sdio_inst_D1      (HPS_SD_DATA[1]),
        .hps_io_hps_io_sdio_inst_D2      (HPS_SD_DATA[2]),
        .hps_io_hps_io_sdio_inst_D3      (HPS_SD_DATA[3]),
        .hps_io_hps_io_sdio_inst_CLK     (HPS_SD_CLK),

        // HPS SPI
        .hps_io_hps_io_spim1_inst_CLK    (HPS_SPIM_CLK),
        .hps_io_hps_io_spim1_inst_MOSI   (HPS_SPIM_MOSI),
        .hps_io_hps_io_spim1_inst_MISO   (HPS_SPIM_MISO),
        .hps_io_hps_io_spim1_inst_SS0    (HPS_SPIM_SS),
        .hps_io_hps_io_spim1_inst_SS1    (),

        // HPS UART
        .hps_io_hps_io_uart0_inst_RX     (HPS_UART_RX),
        .hps_io_hps_io_uart0_inst_TX     (HPS_UART_TX),

        // HPS I2C
        .hps_io_hps_io_i2c0_inst_SDA     (HPS_I2C1_SDAT),
        .hps_io_hps_io_i2c0_inst_SCL     (HPS_I2C1_SCLK),
        .hps_io_hps_io_i2c1_inst_SDA     (HPS_I2C2_SDAT),
        .hps_io_hps_io_i2c1_inst_SCL     (HPS_I2C2_SCLK),

        // En tu QSF existen HPS_GPIO[0] y HPS_GPIO[1].
        // El soc_system solo expone GPIO01, por eso conectamos HPS_GPIO[1].
        .hps_io_hps_io_gpio_inst_GPIO01  (HPS_GPIO[1])
    );

    // --------------------------------------------------------------------
    // Generador de tono + filtros RTL
    // --------------------------------------------------------------------
    ToneGenerator tone_gen (
        .AUD_BCLK    (AUD_BCLK),
        .AUD_DACLRCK (AUD_DACLRCK),
        .reset_n     (KEY[0]),
        .AUD_DACDAT  (AUD_DACDAT),
        .filter_sel  (filter_sel_from_nios)
    );

    // --------------------------------------------------------------------
    // LEDs de debug
    // --------------------------------------------------------------------
    assign LEDR[0] = 1'b1;

    // Switches fisicos
    assign LEDR[3] = SW[0];
    assign LEDR[4] = SW[1];

    // Lo que Nios realmente manda al filtro
    assign LEDR[1] = filter_sel_from_nios[0];
    assign LEDR[2] = filter_sel_from_nios[1];

    //assign LEDR[9:5] = 5'b0;
	 assign LEDR[5] = xck_counter[23];   // Parpadea si AUD_XCK está activo
	 assign LEDR[6] = bclk_counter[22];  // Parpadea si AUD_BCLK está activo
	 assign LEDR[7] = lrck_counter[15];  // Parpadea si AUD_DACLRCK está activo
	 assign LEDR[8] = 1'b0;
	 assign LEDR[9] = 1'b0;
		 
	 
	 // Debug de relojes de audio
		logic [23:0] xck_counter;
		logic [23:0] bclk_counter;
		logic [15:0] lrck_counter;

		always_ff @(posedge AUD_XCK or negedge KEY[0]) begin
			 if (!KEY[0])
				  xck_counter <= 24'd0;
			 else
				  xck_counter <= xck_counter + 24'd1;
		end

		always_ff @(posedge AUD_BCLK or negedge KEY[0]) begin
			 if (!KEY[0])
				  bclk_counter <= 24'd0;
			 else
				  bclk_counter <= bclk_counter + 24'd1;
		end

		always_ff @(posedge AUD_DACLRCK or negedge KEY[0]) begin
			 if (!KEY[0])
				  lrck_counter <= 16'd0;
			 else
				  lrck_counter <= lrck_counter + 16'd1;
		end

endmodule