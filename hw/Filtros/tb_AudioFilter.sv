`timescale 1ns/1ps

module tb_AudioFilter;

    // Señales del testbench
    logic clk;
    logic reset;
    logic sample_valid;
    logic signed [15:0] sample_in;
    logic [1:0] filter_sel;
    logic signed [15:0] sample_out;
    logic sample_out_valid;

    // Instancia del módulo principal
    AudioFilter dut (
        .clk              (clk),
        .reset            (reset),
        .sample_valid     (sample_valid),
        .sample_in        (sample_in),
        .filter_sel       (filter_sel),
        .sample_out       (sample_out),
        .sample_out_valid (sample_out_valid)
    );

    // Generador de reloj
    // Periodo = 10 ns
    always #5 clk = ~clk;

    initial begin
        // Valores iniciales
        clk = 0;
        reset = 1;
        sample_valid = 0;
        sample_in = 16'sd0;
        filter_sel = 2'b00;

        // Mantener reset unos ciclos
        #20;
        reset = 0;

        // =========================
        // Prueba 1: Bypass
        // =========================
        $display("=== Prueba Bypass ===");
        filter_sel = 2'b00;

        send_sample(16'sd1000);
        send_sample(16'sd2000);
        send_sample(-16'sd1000);
        send_sample(-16'sd2000);

        // =========================
        // Prueba 2: Lowpass
        // =========================
        $display("=== Prueba Lowpass ===");
        filter_sel = 2'b01;

        send_sample(16'sd0);
        send_sample(16'sd0);
        send_sample(16'sd8000);
        send_sample(16'sd8000);
        send_sample(16'sd8000);
        send_sample(16'sd8000);
        send_sample(16'sd8000);

        // =========================
        // Prueba 3: Highpass
        // =========================
        $display("=== Prueba Highpass ===");
        filter_sel = 2'b10;

        send_sample(16'sd8000);
        send_sample(-16'sd8000);
        send_sample(16'sd8000);
        send_sample(-16'sd8000);
        send_sample(16'sd8000);
        send_sample(-16'sd8000);

        // =========================
        // Prueba 4: Bass Boost
        // =========================
        $display("=== Prueba Bass Boost ===");
        filter_sel = 2'b11;

        send_sample(16'sd4000);
        send_sample(16'sd4000);
        send_sample(16'sd4000);
        send_sample(16'sd4000);

        // =========================
        // Prueba 5: Saturación positiva
        // =========================
        $display("=== Prueba Saturacion positiva ===");
        filter_sel = 2'b11;

        send_sample(16'sd30000);
        send_sample(16'sd30000);
        send_sample(16'sd30000);
        send_sample(16'sd30000);

        // =========================
        // Prueba 6: Saturación negativa / valores negativos
        // =========================
        $display("=== Prueba valores negativos ===");
        filter_sel = 2'b01;

        send_sample(-16'sd8000);
        send_sample(-16'sd8000);
        send_sample(-16'sd8000);
        send_sample(-16'sd8000);

        #50;
        $stop;
    end

    // Tarea para enviar una muestra válida
    task send_sample(input logic signed [15:0] value);
        begin
            @(negedge clk);
            sample_in = value;
            sample_valid = 1'b1;

            @(negedge clk);
            sample_valid = 1'b0;

            // Esperar un ciclo para observar salida
            @(negedge clk);

            $display("filter_sel=%b | sample_in=%0d | sample_out=%0d | valid=%b",
                     filter_sel, sample_in, sample_out, sample_out_valid);
        end
    endtask

endmodule