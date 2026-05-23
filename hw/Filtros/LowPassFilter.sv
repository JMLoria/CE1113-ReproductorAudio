module LowPassFilter (
	// Señales que vienen de Platform Designer
    input  logic clk,
    input  logic reset,
    input  logic sample_valid,
    input  logic signed [15:0] sample_in,
    output logic signed [31:0] low_out // low de salida que los otros filtros reutilizan
);

    // 32 bits para tratar desbordamiento y no perder información
    logic signed [31:0] low_prev;
    logic signed [31:0] sample_ext; // sample_in extendido a 32 bits   
    logic signed [31:0] diference; // cuanto se aleja el nuevo valor del valor anterior
    logic signed [31:0] adjust;
    logic signed [31:0] low_next;

    always_comb begin
        // >>> es desplazamiento aritmético, conserva el signo
        sample_ext = {{16{sample_in[15]}}, sample_in}; // Extiende sample_in a 32 bits con signo
        diferencia = sample_ext - low_prev; // (x[n] - low[n-1])
        ajuste     = diference >>> 3; // (x[n] - low[n-1]) / 8
        low_next   = low_prev + ajuste; // low[n-1] + (x[n] - low[n-1]) / 8
    end

    always_ff @(posedge clk) begin
        if (reset) begin
            low_prev <= 32'sd0; // Inicializa low_prev a 0 en reset
        end else if (sample_valid) begin
            low_prev <= low_next; // Actualiza low_prev con el nuevo valor calculado
        end
    end

    // Truncar el resultado a 16 bits para la salida
    // Se puede cambiar a saturación para evitar perder información en caso de desbordamiento
    //assign low_out = low_prev[15:0];

    // Salida de 32 bits para que los otros filtros puedan reutilizarla sin perder información
    assign low_out = low_prev; 
    
endmodule