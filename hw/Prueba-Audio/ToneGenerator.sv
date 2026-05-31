module ToneGenerator (
    input  logic AUD_BCLK,
    input  logic AUD_DACLRCK,
    output logic AUD_DACDAT
);

    logic        lrck_d;
    logic [5:0]  bit_pos;
    logic [6:0]  smpl_cnt;
    logic signed [15:0] smpl_val;
    logic [15:0] shift;

    logic signed [31:0] bypass_out;

    // ── Generador de muestra del tono ─────────────────────
    always_comb begin
        smpl_val = (smpl_cnt < 7'd54) ? 16'sh2000 : -16'sh2000;
    end

    // ── Filtro Bypass ─────────────────────────────────────
    ByPassFilter bypass_inst (
        .sample_in  (smpl_val),
        .byPass_out (bypass_out)
    );

    // ── Serializador I2S ──────────────────────────────────
    always_ff @(negedge AUD_BCLK) begin
        lrck_d <= AUD_DACLRCK;

        // Detecta cambio de canal izquierda/derecha
        if (lrck_d != AUD_DACLRCK) begin
            bit_pos <= 6'd0;

            // Aquí ya no cargamos smpl_val directo,
            // sino la salida del filtro bypass
            //shift <= bypass_out[15:0];
				shift <= bypass_out[31:16];

            // Avanzar muestra una vez por frame completo
            if (AUD_DACLRCK == 1'b1) begin
                smpl_cnt <= (smpl_cnt == 7'd108) ? 7'd0 : smpl_cnt + 7'd1;
            end

            AUD_DACDAT <= 1'b0; // primer bit después del cambio, estilo I2S
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

endmodule