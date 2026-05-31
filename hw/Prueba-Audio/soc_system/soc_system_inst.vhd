	component soc_system is
		port (
			clk_clk                      : in    std_logic := 'X'; -- clk
			external_interface_SDAT      : inout std_logic := 'X'; -- SDAT
			external_interface_SCLK      : out   std_logic;        -- SCLK
			external_interface_1_ADCDAT  : in    std_logic := 'X'; -- ADCDAT
			external_interface_1_ADCLRCK : in    std_logic := 'X'; -- ADCLRCK
			external_interface_1_BCLK    : in    std_logic := 'X'; -- BCLK
			external_interface_1_DACDAT  : out   std_logic;        -- DACDAT
			external_interface_1_DACLRCK : in    std_logic := 'X'; -- DACLRCK
			reset_reset_n                : in    std_logic := 'X'  -- reset_n
		);
	end component soc_system;

	u0 : component soc_system
		port map (
			clk_clk                      => CONNECTED_TO_clk_clk,                      --                  clk.clk
			external_interface_SDAT      => CONNECTED_TO_external_interface_SDAT,      --   external_interface.SDAT
			external_interface_SCLK      => CONNECTED_TO_external_interface_SCLK,      --                     .SCLK
			external_interface_1_ADCDAT  => CONNECTED_TO_external_interface_1_ADCDAT,  -- external_interface_1.ADCDAT
			external_interface_1_ADCLRCK => CONNECTED_TO_external_interface_1_ADCLRCK, --                     .ADCLRCK
			external_interface_1_BCLK    => CONNECTED_TO_external_interface_1_BCLK,    --                     .BCLK
			external_interface_1_DACDAT  => CONNECTED_TO_external_interface_1_DACDAT,  --                     .DACDAT
			external_interface_1_DACLRCK => CONNECTED_TO_external_interface_1_DACLRCK, --                     .DACLRCK
			reset_reset_n                => CONNECTED_TO_reset_reset_n                 --                reset.reset_n
		);

