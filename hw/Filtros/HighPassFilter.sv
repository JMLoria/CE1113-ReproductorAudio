// Este filtro es combinacional porque no necesita recordar ningún dato
// Usa la información del filtro LowPassFilter 

module HighPassFilter (

    input  logic signed [15:0] sample_in,
    input logic signed [31:0] low_in,
    output logic signed [31:0] high_out // Se trunca en el principal, para no perder información

);

    always_comb begin

        // Fórmula para el filtro pasa altas
        high_out = {{16{sample_in[15]}}, sample_in} - low_in; // x[n] - low[n]
    end

endmodule