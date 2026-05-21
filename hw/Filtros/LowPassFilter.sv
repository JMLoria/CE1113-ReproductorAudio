module LowPassFilter (
	// Señales que vienen de Platform Designer
    input  logic clk,
    input  logic reset,
    input  logic sample_valid,
    input  logic signed [15:0] sample_in,
    output logic signed [15:0] low_out // low de salida que los otros filtros reutilizan
);

    // 32 bits para tratar desbordamiento 
    logic signed [31:0] low_prev;
    logic signed [31:0] sample_ext; // sample_in extendido a 32 bits   
    logic signed [31:0] diference;
    logic signed [31:0] adjust;
    logic signed [31:0]; low_next;

    always_comb begin
        sample_ext = {{16{sample_in[15]}}, sample_in};
        diferencia = sample_ext - low_prev;
        ajuste     = diferencia >>> 3;
        low_next   = low_prev + ajuste;
    end

    always_ff @(posedge clk) begin
        if (reset) begin
            low_prev <= 32'sd0;
        end else if (sample_valid) begin
            low_prev <= low_next;
        end
    end

    assign low_out = low_prev[15:0];

endmodule