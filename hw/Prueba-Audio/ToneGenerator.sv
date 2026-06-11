module ToneGenerator (
    input  logic        AUD_BCLK,
    input  logic        AUD_DACLRCK,
    input  logic        reset_n,
    output logic        AUD_DACDAT,

    // Esta señal viene del registro escrito por Nios.
    input  logic [1:0]  filter_sel
);

    logic        lrck_d;
    logic [5:0]  bit_pos;
    logic [6:0]  smpl_cnt;
    logic signed [15:0] smpl_val;
    logic [15:0] shift;

    logic signed [15:0] filtered_out;
    logic filtered_valid;

    logic lrck_edge;
    logic sample_valid;

    // Sincronización de filter_sel al reloj de audio
    logic [1:0] filter_sel_meta;
    logic [1:0] filter_sel_sync;

    assign lrck_edge = (lrck_d != AUD_DACLRCK);

    // Pulso de muestra nueva.
    // Se activa solo en uno de los dos cambios de AUD_DACLRCK.
    assign sample_valid = lrck_edge && (AUD_DACLRCK == 1'b1);

    // Sincronizador simple porque filter_sel viene del dominio de CLOCK_50/Nios
    always_ff @(posedge AUD_BCLK or negedge reset_n) begin
        if (!reset_n) begin
            filter_sel_meta <= 2'b00;
            filter_sel_sync <= 2'b00;
        end else begin
            filter_sel_meta <= filter_sel;
            filter_sel_sync <= filter_sel_meta;
        end
    end

    // ── Generador de muestra del tono ─────────────────────
    always_comb begin
        smpl_val = (smpl_cnt < 7'd54) ? 16'sh2000 : -16'sh2000;
    end

    // ── Instancia de filtros ──────────────────────────────
    AudioFilter audio_filter (
        .clk              (AUD_BCLK),
        .reset            (~reset_n),
        .sample_valid     (sample_valid),
        .sample_in        (smpl_val),
        .filter_sel       (filter_sel_sync),
        .sample_out       (filtered_out),
        .sample_out_valid (filtered_valid)
    );

    // ── Serializador I2S ──────────────────────────────────
    always_ff @(negedge AUD_BCLK or negedge reset_n) begin
        if (!reset_n) begin
            lrck_d     <= 1'b0;
            bit_pos    <= 6'd0;
            smpl_cnt   <= 7'd0;
            shift      <= 16'd0;
            AUD_DACDAT <= 1'b0;
        end else begin
            lrck_d <= AUD_DACLRCK;

            // Detecta cambio de canal izquierda/derecha
            if (lrck_d != AUD_DACLRCK) begin
                bit_pos <= 6'd0;

                // Se carga la muestra filtrada
                shift <= filtered_out;

                // Avanzar muestra una vez por frame completo
                if (AUD_DACLRCK == 1'b1) begin
                    smpl_cnt <= (smpl_cnt == 7'd108) ? 7'd0 : smpl_cnt + 7'd1;
                end

                AUD_DACDAT <= 1'b0;
            end else begin
                if (bit_pos < 6'd16) begin
                    AUD_DACDAT <= shift[15];
                    shift      <= shift << 1;
                    bit_pos    <= bit_pos + 6'd1;
                end else begin
                    AUD_DACDAT <= 1'b0;
                    bit_pos    <= bit_pos + 6'd1;
                end
            end
        end
    end

endmodule