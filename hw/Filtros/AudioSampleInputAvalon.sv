module AudioSampleInputAvalon #(
    parameter int FIFO_ADDR_WIDTH = 10, // 2^10 = 1024 muestras
    parameter int FIFO_DEPTH = 1024
)(
    // Reloj/reset del bus Avalon-MM, normalmente CLOCK_50
    input logic clk,
    input logic reset,

    // Interfaz Avalon-MM Slave
    input logic avs_s0_chipselect,
    input logic avs_s0_write,
    input logic avs_s0_read,
    input logic [1:0] avs_s0_address,
    input logic [31:0] avs_s0_writedata,
    output logic [31:0] avs_s0_readdata,

    // Reloj/reset del dominio de audio
    // sample_clock debe conectarse a AUD_BCLK
    // sample_reset es reset activo-alto para este dominio
    input logic sample_clock,
    input logic sample_reset,

    // Interfaz hacia el bloque de audio/filtros
    // sample_request: pulso a 48 kHz para pedir una muestra
    // sample_out: muestra PCM signed 16-bit hacia AudioFilter
    // sample_out_valid: indica que sample_out contiene dato valido
    input logic sample_request,
    output logic signed [15:0] sample_out,
    output logic sample_out_valid,

    // Señales de estado opcionales para debug en top-level
    output logic fifo_full,
    output logic fifo_empty
);

    // Mapa de registros Avalon-MM
    // address 0: write_sample
    // address 1: status
    // address 2: control
    // address 3: id/debug
    //
    // write_sample:
    //   bits [15:0] = muestra PCM signed 16-bit
    //
    // status:
    //   bit 0 = fifo_full
    //   bit 1 = fifo_empty
    //   bit 2 = ready_to_write
    //   bit 3 = overflow_flag
    //   bit 4 = underflow_flag
    //   bits [25:16] = wrusedw aproximado
    //
    // control:
    //   bit 0 = enable
    //
    // id:
    //   0xA5A10001

    localparam logic [31:0] IP_ID = 32'hA5A1_0001;

    logic signed [15:0] fifo_data_in;
    logic signed [15:0] fifo_data_out;

    logic wrreq;
    logic rdreq;

    logic wrfull;
    logic rdempty;

    logic [FIFO_ADDR_WIDTH-1:0] wrusedw;
    logic [FIFO_ADDR_WIDTH-1:0] rdusedw;

    logic enable_reg;
    logic overflow_flag;

    // underflow ocurre en el dominio de sample_clk
    logic underflow_flag_audio;

    // sincronizadores hacia clk para poder leer estado desde Avalon
    logic rdempty_meta;
    logic rdempty_sync;

    logic underflow_meta;
    logic underflow_sync;

    assign fifo_full = wrfull;
    assign fifo_empty = rdempty;

    // Escritura Avalon-MM hacia FIFO
    assign fifo_data_in = avs_s0_writedata[15:0];

    always_comb begin
        wrreq = 1'b0;

        if (avs_s0_chipselect && avs_s0_write) begin
            case (avs_s0_address)
                2'd0: begin
                    // Escribir muestra solo si el FIFO no esta lleno
                    wrreq = !wrfull;
                end

                default: begin
                    wrreq = 1'b0;
                end
            endcase
        end
    end

    // Registros de control y flags en dominio Avalon
    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            enable_reg <= 1'b1;
            overflow_flag <= 1'b0;
        end else begin
            // Si intentan escribir muestra con FIFO lleno, se marca overflow
            if (avs_s0_chipselect && avs_s0_write && avs_s0_address == 2'd0 && wrfull) begin
                overflow_flag <= 1'b1;
            end

            // Registro de control
            if (avs_s0_chipselect && avs_s0_write && avs_s0_address == 2'd2) begin
                enable_reg <= avs_s0_writedata[0];

                // Si writedata[1] = 1, se limpian flags de software
                if (avs_s0_writedata[1]) begin
                    overflow_flag <= 1'b0;
                end
            end
        end
    end

    // Sincronizacion de flags del dominio de audio hacia Avalon
    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            rdempty_meta <= 1'b1;
            rdempty_sync <= 1'b1;
            underflow_meta <= 1'b0;
            underflow_sync <= 1'b0;
        end else begin
            rdempty_meta <= rdempty;
            rdempty_sync <= rdempty_meta;

            underflow_meta <= underflow_flag_audio;
            underflow_sync <= underflow_meta;
        end
    end

    // Lectura Avalon-MM
    always_comb begin
        case (avs_s0_address)
            2'd0: begin
                // Lectura opcional del ultimo sample_out
                avs_s0_readdata = {16'd0, sample_out};
            end

            2'd1: begin
                avs_s0_readdata = {
                    6'd0,
                    wrusedw, // bits [25:16]
                    11'd0,
                    underflow_sync, // bit 4
                    overflow_flag, // bit 3
                    !wrfull, // bit 2 ready_to_write
                    rdempty_sync, // bit 1
                    wrfull // bit 0
                };
            end

            2'd2: begin
                avs_s0_readdata = {31'd0, enable_reg};
            end

            2'd3: begin
                avs_s0_readdata = IP_ID;
            end

            default: begin
                avs_s0_readdata = 32'd0;
            end
        endcase
    end

    // Lectura del FIFO hacia el dominio de audio
    always_comb begin
        rdreq = 1'b0;

        if (enable_reg && sample_request && !rdempty) begin
            rdreq = 1'b1;
        end
    end

    always_ff @(posedge sample_clock or posedge sample_reset) begin
        if (sample_reset) begin
            sample_out <= 16'sd0;
            sample_out_valid <= 1'b0;
            underflow_flag_audio <= 1'b0;
        end else begin
            sample_out_valid <= 1'b0;

            if (sample_request) begin
                if (enable_reg && !rdempty) begin
                    sample_out <= fifo_data_out;
                    sample_out_valid <= 1'b1;
                end else begin
                    sample_out <= 16'sd0;
                    sample_out_valid <= 1'b0;
                    underflow_flag_audio <= 1'b1;
                end
            end
        end
    end

    // FIFO doble reloj
    dcfifo #(
        .intended_device_family ("Cyclone V"),
        .lpm_numwords (FIFO_DEPTH),
        .lpm_showahead ("ON"),
        .lpm_type ("dcfifo"),
        .lpm_width (16),
        .lpm_widthu (FIFO_ADDR_WIDTH),
        .overflow_checking ("ON"),
        .underflow_checking ("ON"),
        .rdsync_delaypipe (5),
        .wrsync_delaypipe (5),
        .use_eab ("ON")
    ) sample_fifo (
        .data (fifo_data_in),
        .wrreq (wrreq),
        .rdreq (rdreq),
        .wrclk (clk),
        .rdclk (sample_clock),
        .aclr (reset | sample_reset),
        .q (fifo_data_out),
        .wrfull (wrfull),
        .rdempty (rdempty),
        .wrusedw (wrusedw),
        .rdusedw (rdusedw)
    );

endmodule