	component soc_system is
		port (
			audio_BCLK                      : in    std_logic                     := 'X';             -- BCLK
			audio_DACDAT                    : out   std_logic;                                        -- DACDAT
			audio_DACLRCK                   : in    std_logic                     := 'X';             -- DACLRCK
			audio_config_SDAT               : inout std_logic                     := 'X';             -- SDAT
			audio_config_SCLK               : out   std_logic;                                        -- SCLK
			buttons_export                  : in    std_logic_vector(3 downto 0)  := (others => 'X'); -- export
			clk_clk                         : in    std_logic                     := 'X';             -- clk
			hex_hex0                        : out   std_logic_vector(6 downto 0);                     -- hex0
			hex_hex1                        : out   std_logic_vector(6 downto 0);                     -- hex1
			hex_hex2                        : out   std_logic_vector(6 downto 0);                     -- hex2
			hex_hex3                        : out   std_logic_vector(6 downto 0);                     -- hex3
			hex_hex4                        : out   std_logic_vector(6 downto 0);                     -- hex4
			hex_hex5                        : out   std_logic_vector(6 downto 0);                     -- hex5
			hps_io_hps_io_emac1_inst_TX_CLK : out   std_logic;                                        -- hps_io_emac1_inst_TX_CLK
			hps_io_hps_io_emac1_inst_TXD0   : out   std_logic;                                        -- hps_io_emac1_inst_TXD0
			hps_io_hps_io_emac1_inst_TXD1   : out   std_logic;                                        -- hps_io_emac1_inst_TXD1
			hps_io_hps_io_emac1_inst_TXD2   : out   std_logic;                                        -- hps_io_emac1_inst_TXD2
			hps_io_hps_io_emac1_inst_TXD3   : out   std_logic;                                        -- hps_io_emac1_inst_TXD3
			hps_io_hps_io_emac1_inst_RXD0   : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RXD0
			hps_io_hps_io_emac1_inst_MDIO   : inout std_logic                     := 'X';             -- hps_io_emac1_inst_MDIO
			hps_io_hps_io_emac1_inst_MDC    : out   std_logic;                                        -- hps_io_emac1_inst_MDC
			hps_io_hps_io_emac1_inst_RX_CTL : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RX_CTL
			hps_io_hps_io_emac1_inst_TX_CTL : out   std_logic;                                        -- hps_io_emac1_inst_TX_CTL
			hps_io_hps_io_emac1_inst_RX_CLK : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RX_CLK
			hps_io_hps_io_emac1_inst_RXD1   : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RXD1
			hps_io_hps_io_emac1_inst_RXD2   : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RXD2
			hps_io_hps_io_emac1_inst_RXD3   : in    std_logic                     := 'X';             -- hps_io_emac1_inst_RXD3
			hps_io_hps_io_sdio_inst_CMD     : inout std_logic                     := 'X';             -- hps_io_sdio_inst_CMD
			hps_io_hps_io_sdio_inst_D0      : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D0
			hps_io_hps_io_sdio_inst_D1      : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D1
			hps_io_hps_io_sdio_inst_CLK     : out   std_logic;                                        -- hps_io_sdio_inst_CLK
			hps_io_hps_io_sdio_inst_D2      : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D2
			hps_io_hps_io_sdio_inst_D3      : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D3
			hps_io_hps_io_spim1_inst_SS1    : out   std_logic;                                        -- hps_io_spim1_inst_SS1
			hps_io_hps_io_spim1_inst_CLK    : out   std_logic;                                        -- hps_io_spim1_inst_CLK
			hps_io_hps_io_spim1_inst_MOSI   : out   std_logic;                                        -- hps_io_spim1_inst_MOSI
			hps_io_hps_io_spim1_inst_MISO   : in    std_logic                     := 'X';             -- hps_io_spim1_inst_MISO
			hps_io_hps_io_spim1_inst_SS0    : out   std_logic;                                        -- hps_io_spim1_inst_SS0
			hps_io_hps_io_uart0_inst_RX     : in    std_logic                     := 'X';             -- hps_io_uart0_inst_RX
			hps_io_hps_io_uart0_inst_TX     : out   std_logic;                                        -- hps_io_uart0_inst_TX
			hps_io_hps_io_i2c0_inst_SDA     : inout std_logic                     := 'X';             -- hps_io_i2c0_inst_SDA
			hps_io_hps_io_i2c0_inst_SCL     : inout std_logic                     := 'X';             -- hps_io_i2c0_inst_SCL
			hps_io_hps_io_i2c1_inst_SDA     : inout std_logic                     := 'X';             -- hps_io_i2c1_inst_SDA
			hps_io_hps_io_i2c1_inst_SCL     : inout std_logic                     := 'X';             -- hps_io_i2c1_inst_SCL
			hps_io_hps_io_gpio_inst_GPIO01  : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO01
			leds_export                     : out   std_logic_vector(9 downto 0);                     -- export
			memory_mem_a                    : out   std_logic_vector(14 downto 0);                    -- mem_a
			memory_mem_ba                   : out   std_logic_vector(2 downto 0);                     -- mem_ba
			memory_mem_ck                   : out   std_logic;                                        -- mem_ck
			memory_mem_ck_n                 : out   std_logic;                                        -- mem_ck_n
			memory_mem_cke                  : out   std_logic;                                        -- mem_cke
			memory_mem_cs_n                 : out   std_logic;                                        -- mem_cs_n
			memory_mem_ras_n                : out   std_logic;                                        -- mem_ras_n
			memory_mem_cas_n                : out   std_logic;                                        -- mem_cas_n
			memory_mem_we_n                 : out   std_logic;                                        -- mem_we_n
			memory_mem_reset_n              : out   std_logic;                                        -- mem_reset_n
			memory_mem_dq                   : inout std_logic_vector(31 downto 0) := (others => 'X'); -- mem_dq
			memory_mem_dqs                  : inout std_logic_vector(3 downto 0)  := (others => 'X'); -- mem_dqs
			memory_mem_dqs_n                : inout std_logic_vector(3 downto 0)  := (others => 'X'); -- mem_dqs_n
			memory_mem_odt                  : out   std_logic;                                        -- mem_odt
			memory_mem_dm                   : out   std_logic_vector(3 downto 0);                     -- mem_dm
			memory_oct_rzqin                : in    std_logic                     := 'X';             -- oct_rzqin
			switches_export                 : in    std_logic_vector(9 downto 0)  := (others => 'X'); -- export
			audio_clk_clk                   : out   std_logic                                         -- clk
		);
	end component soc_system;

	u0 : component soc_system
		port map (
			audio_BCLK                      => CONNECTED_TO_audio_BCLK,                      --        audio.BCLK
			audio_DACDAT                    => CONNECTED_TO_audio_DACDAT,                    --             .DACDAT
			audio_DACLRCK                   => CONNECTED_TO_audio_DACLRCK,                   --             .DACLRCK
			audio_config_SDAT               => CONNECTED_TO_audio_config_SDAT,               -- audio_config.SDAT
			audio_config_SCLK               => CONNECTED_TO_audio_config_SCLK,               --             .SCLK
			buttons_export                  => CONNECTED_TO_buttons_export,                  --      buttons.export
			clk_clk                         => CONNECTED_TO_clk_clk,                         --          clk.clk
			hex_hex0                        => CONNECTED_TO_hex_hex0,                        --          hex.hex0
			hex_hex1                        => CONNECTED_TO_hex_hex1,                        --             .hex1
			hex_hex2                        => CONNECTED_TO_hex_hex2,                        --             .hex2
			hex_hex3                        => CONNECTED_TO_hex_hex3,                        --             .hex3
			hex_hex4                        => CONNECTED_TO_hex_hex4,                        --             .hex4
			hex_hex5                        => CONNECTED_TO_hex_hex5,                        --             .hex5
			hps_io_hps_io_emac1_inst_TX_CLK => CONNECTED_TO_hps_io_hps_io_emac1_inst_TX_CLK, --       hps_io.hps_io_emac1_inst_TX_CLK
			hps_io_hps_io_emac1_inst_TXD0   => CONNECTED_TO_hps_io_hps_io_emac1_inst_TXD0,   --             .hps_io_emac1_inst_TXD0
			hps_io_hps_io_emac1_inst_TXD1   => CONNECTED_TO_hps_io_hps_io_emac1_inst_TXD1,   --             .hps_io_emac1_inst_TXD1
			hps_io_hps_io_emac1_inst_TXD2   => CONNECTED_TO_hps_io_hps_io_emac1_inst_TXD2,   --             .hps_io_emac1_inst_TXD2
			hps_io_hps_io_emac1_inst_TXD3   => CONNECTED_TO_hps_io_hps_io_emac1_inst_TXD3,   --             .hps_io_emac1_inst_TXD3
			hps_io_hps_io_emac1_inst_RXD0   => CONNECTED_TO_hps_io_hps_io_emac1_inst_RXD0,   --             .hps_io_emac1_inst_RXD0
			hps_io_hps_io_emac1_inst_MDIO   => CONNECTED_TO_hps_io_hps_io_emac1_inst_MDIO,   --             .hps_io_emac1_inst_MDIO
			hps_io_hps_io_emac1_inst_MDC    => CONNECTED_TO_hps_io_hps_io_emac1_inst_MDC,    --             .hps_io_emac1_inst_MDC
			hps_io_hps_io_emac1_inst_RX_CTL => CONNECTED_TO_hps_io_hps_io_emac1_inst_RX_CTL, --             .hps_io_emac1_inst_RX_CTL
			hps_io_hps_io_emac1_inst_TX_CTL => CONNECTED_TO_hps_io_hps_io_emac1_inst_TX_CTL, --             .hps_io_emac1_inst_TX_CTL
			hps_io_hps_io_emac1_inst_RX_CLK => CONNECTED_TO_hps_io_hps_io_emac1_inst_RX_CLK, --             .hps_io_emac1_inst_RX_CLK
			hps_io_hps_io_emac1_inst_RXD1   => CONNECTED_TO_hps_io_hps_io_emac1_inst_RXD1,   --             .hps_io_emac1_inst_RXD1
			hps_io_hps_io_emac1_inst_RXD2   => CONNECTED_TO_hps_io_hps_io_emac1_inst_RXD2,   --             .hps_io_emac1_inst_RXD2
			hps_io_hps_io_emac1_inst_RXD3   => CONNECTED_TO_hps_io_hps_io_emac1_inst_RXD3,   --             .hps_io_emac1_inst_RXD3
			hps_io_hps_io_sdio_inst_CMD     => CONNECTED_TO_hps_io_hps_io_sdio_inst_CMD,     --             .hps_io_sdio_inst_CMD
			hps_io_hps_io_sdio_inst_D0      => CONNECTED_TO_hps_io_hps_io_sdio_inst_D0,      --             .hps_io_sdio_inst_D0
			hps_io_hps_io_sdio_inst_D1      => CONNECTED_TO_hps_io_hps_io_sdio_inst_D1,      --             .hps_io_sdio_inst_D1
			hps_io_hps_io_sdio_inst_CLK     => CONNECTED_TO_hps_io_hps_io_sdio_inst_CLK,     --             .hps_io_sdio_inst_CLK
			hps_io_hps_io_sdio_inst_D2      => CONNECTED_TO_hps_io_hps_io_sdio_inst_D2,      --             .hps_io_sdio_inst_D2
			hps_io_hps_io_sdio_inst_D3      => CONNECTED_TO_hps_io_hps_io_sdio_inst_D3,      --             .hps_io_sdio_inst_D3
			hps_io_hps_io_spim1_inst_SS1    => CONNECTED_TO_hps_io_hps_io_spim1_inst_SS1,    --             .hps_io_spim1_inst_SS1
			hps_io_hps_io_spim1_inst_CLK    => CONNECTED_TO_hps_io_hps_io_spim1_inst_CLK,    --             .hps_io_spim1_inst_CLK
			hps_io_hps_io_spim1_inst_MOSI   => CONNECTED_TO_hps_io_hps_io_spim1_inst_MOSI,   --             .hps_io_spim1_inst_MOSI
			hps_io_hps_io_spim1_inst_MISO   => CONNECTED_TO_hps_io_hps_io_spim1_inst_MISO,   --             .hps_io_spim1_inst_MISO
			hps_io_hps_io_spim1_inst_SS0    => CONNECTED_TO_hps_io_hps_io_spim1_inst_SS0,    --             .hps_io_spim1_inst_SS0
			hps_io_hps_io_uart0_inst_RX     => CONNECTED_TO_hps_io_hps_io_uart0_inst_RX,     --             .hps_io_uart0_inst_RX
			hps_io_hps_io_uart0_inst_TX     => CONNECTED_TO_hps_io_hps_io_uart0_inst_TX,     --             .hps_io_uart0_inst_TX
			hps_io_hps_io_i2c0_inst_SDA     => CONNECTED_TO_hps_io_hps_io_i2c0_inst_SDA,     --             .hps_io_i2c0_inst_SDA
			hps_io_hps_io_i2c0_inst_SCL     => CONNECTED_TO_hps_io_hps_io_i2c0_inst_SCL,     --             .hps_io_i2c0_inst_SCL
			hps_io_hps_io_i2c1_inst_SDA     => CONNECTED_TO_hps_io_hps_io_i2c1_inst_SDA,     --             .hps_io_i2c1_inst_SDA
			hps_io_hps_io_i2c1_inst_SCL     => CONNECTED_TO_hps_io_hps_io_i2c1_inst_SCL,     --             .hps_io_i2c1_inst_SCL
			hps_io_hps_io_gpio_inst_GPIO01  => CONNECTED_TO_hps_io_hps_io_gpio_inst_GPIO01,  --             .hps_io_gpio_inst_GPIO01
			leds_export                     => CONNECTED_TO_leds_export,                     --         leds.export
			memory_mem_a                    => CONNECTED_TO_memory_mem_a,                    --       memory.mem_a
			memory_mem_ba                   => CONNECTED_TO_memory_mem_ba,                   --             .mem_ba
			memory_mem_ck                   => CONNECTED_TO_memory_mem_ck,                   --             .mem_ck
			memory_mem_ck_n                 => CONNECTED_TO_memory_mem_ck_n,                 --             .mem_ck_n
			memory_mem_cke                  => CONNECTED_TO_memory_mem_cke,                  --             .mem_cke
			memory_mem_cs_n                 => CONNECTED_TO_memory_mem_cs_n,                 --             .mem_cs_n
			memory_mem_ras_n                => CONNECTED_TO_memory_mem_ras_n,                --             .mem_ras_n
			memory_mem_cas_n                => CONNECTED_TO_memory_mem_cas_n,                --             .mem_cas_n
			memory_mem_we_n                 => CONNECTED_TO_memory_mem_we_n,                 --             .mem_we_n
			memory_mem_reset_n              => CONNECTED_TO_memory_mem_reset_n,              --             .mem_reset_n
			memory_mem_dq                   => CONNECTED_TO_memory_mem_dq,                   --             .mem_dq
			memory_mem_dqs                  => CONNECTED_TO_memory_mem_dqs,                  --             .mem_dqs
			memory_mem_dqs_n                => CONNECTED_TO_memory_mem_dqs_n,                --             .mem_dqs_n
			memory_mem_odt                  => CONNECTED_TO_memory_mem_odt,                  --             .mem_odt
			memory_mem_dm                   => CONNECTED_TO_memory_mem_dm,                   --             .mem_dm
			memory_oct_rzqin                => CONNECTED_TO_memory_oct_rzqin,                --             .oct_rzqin
			switches_export                 => CONNECTED_TO_switches_export,                 --     switches.export
			audio_clk_clk                   => CONNECTED_TO_audio_clk_clk                    --    audio_clk.clk
		);

