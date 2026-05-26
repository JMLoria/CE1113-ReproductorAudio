`timescale 1ns/1ps
module tb_AudioFilter;

    // =========================================================================
    // Señales del testbench
    // =========================================================================
    logic        clk;
    logic        reset;
    logic        sample_valid;
    logic signed [15:0] sample_in;
    logic [1:0]  filter_sel;
    logic signed [15:0] sample_out;
    logic        sample_out_valid;

    // Variables para captura de salida
    logic signed [15:0] captured_out;
    logic               captured_valid;

    // Contadores globales de PASS/FAIL
    int pass_count;
    int fail_count;

    // =========================================================================
    // Instancia del DUT
    // =========================================================================
    AudioFilter dut (
        .clk              (clk),
        .reset            (reset),
        .sample_valid     (sample_valid),
        .sample_in        (sample_in),
        .filter_sel       (filter_sel),
        .sample_out       (sample_out),
        .sample_out_valid (sample_out_valid)
    );

    // =========================================================================
    // Generador de reloj — periodo 10 ns
    // =========================================================================
    always #5 clk = ~clk;

    // =========================================================================
    // Tarea: reset limpio
    //   - Activa reset durante 4 ciclos completos para garantizar que
    //     low_prev quede en 0 sin importar el estado previo.
    //   - Termina en un negedge para sincronizar con send_sample.
    // =========================================================================
    task do_reset();
        begin
            @(negedge clk);       // Sincronizar al negedge antes de cambios
            reset        = 1;
            sample_valid = 0;
            sample_in    = 16'sd0;
            repeat(4) @(posedge clk);  // 4 flancos positivos con reset=1
            @(negedge clk);            // Terminar en negedge
            reset = 0;
        end
    endtask

    // =========================================================================
    // Tarea: enviar una muestra y capturar la salida
    //
    //   Diagrama de temporización:
    //
    //   negedge ──► sample_in=value, sample_valid=1
    //   posedge ──► FF actualiza low_prev (lógica combinacional re-evalúa)
    //   negedge ──► sample_valid=0, capturar sample_out (ya estabilizado)
    //   negedge ──► mostrar resultado
    //
    //   Se captura en el negedge POSTERIOR al posedge de actualización.
    //   Esto evita el problema de leer valores NBA aún no propagados.
    // =========================================================================
    task send_sample(input logic signed [15:0] value);
        begin
            @(negedge clk);
            sample_in    = value;
            sample_valid = 1'b1;

            // El posedge entre estos dos negedges actualiza low_prev en el FF.
            // Al llegar al siguiente negedge, la lógica combinacional
            // (bass/high/bypass) ya tiene un ciclo completo para estabilizarse.
            @(negedge clk);
            sample_valid   = 1'b0;
            captured_out   = sample_out;    // low_prev ya actualizado
            captured_valid = 1'b1;          // El sample fue válido en el posedge anterior

            @(negedge clk);
            $display("  filter_sel=%b | sample_in=%7d | sample_out=%7d | valid=%b",
                     filter_sel, value, captured_out, captured_valid);
        end
    endtask

    // =========================================================================
    // Tarea: enviar muestra y verificar valor esperado exacto
    // =========================================================================
    task check_exact(
        input logic signed [15:0] value,
        input logic signed [15:0] expected
    );
        begin
            send_sample(value);
            if (captured_out === expected) begin
                $display("    [PASS] out=%0d == esperado=%0d", captured_out, expected);
                pass_count++;
            end else begin
                $display("    [FAIL] out=%0d != esperado=%0d", captured_out, expected);
                fail_count++;
            end
        end
    endtask

    // =========================================================================
    // Tarea: verificar que la salida esté saturada
    //   - La primera muestra desde reset puede NO saturar (low_prev=0).
    //   - Se envía 'warm_up' muestras previas sin verificar para acumular
    //     el estado del LowPassFilter, luego se verifica 'verify_count' veces.
    // =========================================================================
    task check_saturation(
        input logic signed [15:0] value,
        input logic signed [15:0] expected_sat,
        input int warm_up,
        input int verify_count
    );
        integer i;
        begin
            // Ciclos de calentamiento: acumular estado sin verificar
            for (i = 0; i < warm_up; i++) begin
                send_sample(value);
                $display("    [WARM-UP %0d] out=%0d", i+1, captured_out);
            end
            // Ciclos de verificación: la saturación debe ser constante
            for (i = 0; i < verify_count; i++) begin
                send_sample(value);
                if (captured_out === expected_sat) begin
                    $display("    [PASS] Saturacion correcta: %0d", captured_out);
                    pass_count++;
                end else begin
                    $display("    [FAIL] Esperado=%0d, Obtenido=%0d", expected_sat, captured_out);
                    fail_count++;
                end
            end
        end
    endtask

    // =========================================================================
    // Bloque principal de pruebas
    // =========================================================================
    initial begin
        // Inicialización
        clk          = 0;
        reset        = 1;
        sample_valid = 0;
        sample_in    = 16'sd0;
        filter_sel   = 2'b00;
        captured_out   = 16'sd0;
        captured_valid = 1'b0;
        pass_count   = 0;
        fail_count   = 0;

        // =====================================================================
        // PRUEBA 1: Bypass
        //   filter_sel = 2'b00
        //   Esperado: sample_out == sample_in exactamente (sin filtrado)
        // =====================================================================
        do_reset();
        $display("\n=== PRUEBA 1: Bypass — esperado out == in ===");
        filter_sel = 2'b00;
        check_exact( 16'sd1000,  16'sd1000);
        check_exact( 16'sd2000,  16'sd2000);
        check_exact(-16'sd1000, -16'sd1000);
        check_exact(-16'sd2000, -16'sd2000);
        check_exact( 16'sd0,     16'sd0);

        // =====================================================================
        // PRUEBA 2: Lowpass — escalón positivo 0 → 8000
        //   filter_sel = 2'b01
        //   Esperado: salida sube lentamente hacia 8000 (convergencia exponencial)
        //   Fórmula: low[n] = low[n-1] + (8000 - low[n-1]) / 8
        // =====================================================================
        do_reset();
        $display("\n=== PRUEBA 2: Lowpass escalon 0->8000 (debe subir hacia 8000) ===");
        filter_sel = 2'b01;
        // Valores esperados calculados manualmente con desplazamiento aritmético
        check_exact(16'sd8000, 16'sd1000);   // low: 0    + 8000/8   = 1000
        check_exact(16'sd8000, 16'sd1875);   // low: 1000 + 7000/8   = 1875
        check_exact(16'sd8000, 16'sd2640);   // low: 1875 + 6125/8   = 2640  (>>3 trunca a -inf)
        check_exact(16'sd8000, 16'sd3310);   // low: 2640 + 5360/8   = 3310
        check_exact(16'sd8000, 16'sd3896);   // low: 3310 + 4690/8   = 3896
        check_exact(16'sd8000, 16'sd4409);   // low: 3896 + 4104/8   = 4409  (>>3 trunca)
        check_exact(16'sd8000, 16'sd4857);   // low: 4409 + 3591/8   = 4857  (>>3 trunca)
        check_exact(16'sd8000, 16'sd5249);   // low: 4857 + 3143/8   = 5249  (>>3 trunca)
        check_exact(16'sd8000, 16'sd5592);   // low: 5249 + 2751/8   = 5592  (>>3 trunca)
        check_exact(16'sd8000, 16'sd5893);   // low: 5592 + 2408/8   = 5893  (>>3 trunca)

        // =====================================================================
        // PRUEBA 3: Highpass — señal alternante ±8000
        //   filter_sel = 2'b10
        //   high[n] = sample_in - low_prev (post-actualización)
        //   Esperado: salida oscila cerca de ±8000, con componente DC que decae
        // =====================================================================
        do_reset();
        $display("\n=== PRUEBA 3: Highpass +-8000 (debe oscilar cerca de +-8000) ===");
        filter_sel = 2'b10;
        // low_prev post-clk | high = sample_in - low_prev_new
        check_exact( 16'sd8000,  16'sd7000);  // low=1000,  high=8000-1000=7000
        check_exact(-16'sd8000, -16'sd7875);  // low=-125,  high=-8000-(-125)=-7875
        check_exact( 16'sd8000,  16'sd7110);  // low=890,   high=8000-890=7110
        check_exact(-16'sd8000, -16'sd7778);  // low=-222,  high=-8000-(-222)=-7778
        check_exact( 16'sd8000,  16'sd7195);  // low=805,   high=8000-805=7195
        check_exact(-16'sd8000, -16'sd7704);  // low=-296,  high=-8000-(-296)=-7704

        // =====================================================================
        // PRUEBA 4: Bass Boost — señal constante 4000
        //   filter_sel = 2'b11
        //   bass[n] = sample_in + low_prev/2 (post-actualización)
        //   Esperado: salida > 4000, sube progresivamente hasta ~6000
        // =====================================================================
        do_reset();
        $display("\n=== PRUEBA 4: Bass Boost constante 4000 (debe superar 4000 y subir) ===");
        filter_sel = 2'b11;
        check_exact(16'sd4000, 16'sd4250);  // low=500,  bass=4000+250=4250
        check_exact(16'sd4000, 16'sd4468);  // low=937,  bass=4000+468=4468
        check_exact(16'sd4000, 16'sd4659);  // low=1319, bass=4000+659=4659
        check_exact(16'sd4000, 16'sd4827);  // low=1654, bass=4000+827=4827
        check_exact(16'sd4000, 16'sd4973);  // low=1947, bass=4000+973=4973
        check_exact(16'sd4000, 16'sd5101);  // low=2203, bass=4000+1101=5101

        // =====================================================================
        // PRUEBA 5: Saturación positiva
        //   filter_sel = 2'b11 (Bass Boost con 30000)
        //   La primera muestra desde reset NO satura (low_prev=0 → out=31875).
        //   warm_up=1 acumula estado, luego se verifica saturación en las sig. 3.
        // =====================================================================
        do_reset();
        $display("\n=== PRUEBA 5: Saturacion positiva (32767) — warm-up 1, verif. 3 ===");
        filter_sel = 2'b11;
        check_saturation(16'sd30000, 16'sd32767, 1, 3);

        // =====================================================================
        // PRUEBA 6: Saturación negativa
        //   filter_sel = 2'b11 (Bass Boost con -30000)
        //   Igual que Prueba 5: warm_up=1, verif. 3
        //   Muestra 1: low_prev=0 → bass=-30000-1875=-31875 (no satura)
        //   Muestra 2: low_prev=-7031 → bass=-30000-3515=-33515 → satura a -32768
        // =====================================================================
        do_reset();
        $display("\n=== PRUEBA 6: Saturacion negativa (-32768) — warm-up 1, verif. 3 ===");
        filter_sel = 2'b11;
        check_saturation(-16'sd30000, -16'sd32768, 1, 3);

        // =====================================================================
        // PRUEBA 7: Lowpass — escalón negativo 0 → -8000
        //   filter_sel = 2'b01
        //   Espejo exacto de Prueba 2 con signo invertido
        // =====================================================================
        do_reset();
        $display("\n=== PRUEBA 7: Lowpass escalon 0->-8000 (debe bajar hacia -8000) ===");
        filter_sel = 2'b01;
        check_exact(-16'sd8000, -16'sd1000);
        check_exact(-16'sd8000, -16'sd1875);
        check_exact(-16'sd8000, -16'sd2641);  // trunca hacia -inf: -6125>>>3 = -766
        check_exact(-16'sd8000, -16'sd3311);
        check_exact(-16'sd8000, -16'sd3898);
        check_exact(-16'sd8000, -16'sd4411);
        check_exact(-16'sd8000, -16'sd4860);
        check_exact(-16'sd8000, -16'sd5253);

        // =====================================================================
        // PRUEBA 8: Cambio de filtro sin reset
        //   Verifica que el mux de selección conmuta correctamente
        //   entre filtros en el mismo ciclo
        // =====================================================================
        do_reset();
        $display("\n=== PRUEBA 8: Cambio de filtro en caliente ===");
        // Acumular algo de estado en LowPass primero
        filter_sel = 2'b01;
        send_sample(16'sd4000);  // low_prev se actualiza
        send_sample(16'sd4000);

        // Ahora cambiar a bypass: debe salir sample_in directamente
        filter_sel = 2'b00;
        $display("  (cambiando a Bypass)");
        check_exact(16'sd1234, 16'sd1234);
        check_exact(-16'sd5678, -16'sd5678);

        // =====================================================================
        // Resumen final
        // =====================================================================
        #50;
        $display("\n================================================");
        $display("  RESUMEN: %0d PASS | %0d FAIL | %0d TOTAL",
                 pass_count, fail_count, pass_count + fail_count);
        $display("================================================\n");
        $stop;
    end

endmodule