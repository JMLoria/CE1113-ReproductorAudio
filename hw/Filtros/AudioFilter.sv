module AudioFilter( 

    // Señales que vienen de Platform Designer
    input  logic clk,
    input  logic reset,
    input  logic sample_valid,
    input  logic signed [15:0] sample_in,
    input logic [1:0] filter_sel, // elección del filtro
    output logic signed [15:0] sample_out,
    output logic sample_out_valid // Salida válida
 );
    // Salidas de los filtros
    logic signed [31:0] low;   // cable interno que comparten todos los filtros
    logic signed [31:0] high;
    logic signed [31:0] bass;
    logic signed [31:0] bypass;

    // Filtro seleccionado para la salida
    logic signed [31:0] selected_filter;

    // Instancia del lowpass — calcula low
    LowPassFilter f_low (
        .clk (clk),
        .reset (reset),
        .sample_valid (sample_valid),
        .sample_in (sample_in),
        .low_out (low)
    );

    // Instancia del highpass
    HighPassFilter f_high (
        .sample_in (sample_in),
        .low_in (low),         
        .high_out (high)
    );

    // Instancia del bass boost 
    BassBoostFilter f_bass (
        .sample_in (sample_in),
        .low_in (low),
        .bassBoost_out (bass)
    );

    // Instancia del by pass
    ByPassFilter f_bypass (
        .sample_in (sample_in),
        .byPass_out (bypass)
    );

    // Se usan dos switches para elegir el filtro y así evitar que se enciendan varios switches a la vez por error
    always_comb begin
        case (filter_sel) // SW[1:0]
            2'b00: selected_filter = bypass; // SW[0] = 0, SW[1] = 0 
            2'b01: selected_filter = low;   // SW[0] = 1, SW[1] = 0
            2'b10: selected_filter = high;   // SW[0] = 0, SW[1] = 1
            2'b11: selected_filter = bass;   // SW[0] = 1, SW[1] = 1
            default: selected_filter = bypass;
        endcase
    end

    // Saturar para que no sobrepasen slo 16 bits, en cuanto a menos o a más
    always_comb begin
        if (reset)
            sample_out = 16'sd0; // Valor en cero durante reset
        else if (selected_filter > 32'sd32767) // Que no sobrepase el máximo
            sample_out = 16'sd32767;
        else if (selected_filter < -32'sd32768) // Que no sobrepase el mínimo
            sample_out = 16'sh8000; // -32768
        else
            sample_out = selected_filter[15:0];
    end
	 
	 always @(posedge clk) begin
		 if (sample_valid && filter_sel == 2'b11) begin
			  $display("DEBUG BASS | sample_in=%0d | low=%0d | bass=%0d | selected=%0d | out=%0d",
						  sample_in, low, bass, selected_filter, sample_out);
		 end
end

    assign sample_out_valid = sample_valid;


endmodule