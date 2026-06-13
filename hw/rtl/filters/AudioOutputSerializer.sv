module AudioOutputSerializer (
    input  logic        AUD_BCLK,
    input  logic        AUD_DACLRCK,
    input  logic        reset_n,

    // Muestra paralela que viene del FIFO / SampleInput
    input  logic signed [15:0] sample_in,
    input  logic               sample_in_valid,

    // Seleccion de filtro desde Nios
    input  logic [1:0] filter_sel,

    // Pulso que pide una muestra nueva al FIFO
    output logic sample_request,

    // Salida serial hacia el codec
    output logic AUD_DACDAT
);

    // ------------------------------------------------------------
    // Generacion de sample_request
    // Una solicitud por frame de audio, usando AUD_DACLRCK.
    // Se genera en dominio AUD_BCLK.
    // ------------------------------------------------------------
    logic daclrck_req_d;

    always_ff @(posedge AUD_BCLK or negedge reset_n) begin
        if (!reset_n) begin
            daclrck_req_d  <= 1'b0;
            sample_request <= 1'b0;
        end else begin
            daclrck_req_d  <= AUD_DACLRCK;

            // Pulso de 1 ciclo de BCLK cuando DACLRCK sube
            sample_request <= (daclrck_req_d != AUD_DACLRCK) &&
                              (AUD_DACLRCK == 1'b1);
        end
    end

    // ------------------------------------------------------------
    // Filtro de audio
    // ------------------------------------------------------------
    logic signed [15:0] filtered_out;
    logic               filtered_valid;

    AudioFilter audio_filter (
        .clk              (AUD_BCLK),
        .reset            (~reset_n),
        .sample_valid     (sample_in_valid),
        .sample_in        (sample_in),
        .filter_sel       (filter_sel),
        .sample_out       (filtered_out),
        .sample_out_valid (filtered_valid)
    );

    // Guardamos la ultima muestra filtrada valida.
    logic signed [15:0] current_sample;

    always_ff @(posedge AUD_BCLK or negedge reset_n) begin
        if (!reset_n) begin
            current_sample <= 16'sd0;
        end else begin
            if (filtered_valid) begin
                current_sample <= filtered_out;
            end
        end
    end

    // ------------------------------------------------------------
    // Serializador I2S hacia AUD_DACDAT
    // ------------------------------------------------------------
    logic       daclrck_ser_d;
    logic [5:0] bit_pos;
    logic [15:0] shift_reg;

    always_ff @(negedge AUD_BCLK or negedge reset_n) begin
        if (!reset_n) begin
            daclrck_ser_d <= 1'b0;
            bit_pos       <= 6'd0;
            shift_reg     <= 16'd0;
            AUD_DACDAT    <= 1'b0;
        end else begin
            daclrck_ser_d <= AUD_DACLRCK;

            // Cambio de canal/frame
            if (daclrck_ser_d != AUD_DACLRCK) begin
                bit_pos    <= 6'd0;
                shift_reg  <= current_sample;
                AUD_DACDAT <= 1'b0;   // retardo de 1 bit estilo I2S
            end else begin
                if (bit_pos < 6'd16) begin
                    AUD_DACDAT <= shift_reg[15];
                    shift_reg  <= {shift_reg[14:0], 1'b0};
                    bit_pos    <= bit_pos + 6'd1;
                end else begin
                    AUD_DACDAT <= 1'b0;
                    bit_pos    <= bit_pos + 6'd1;
                end
            end
        end
    end

endmodule