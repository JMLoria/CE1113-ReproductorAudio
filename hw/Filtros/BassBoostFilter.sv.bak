// Este filtro es combinacional porque no necesita recordar ningún dato
// Usa la información del filtro LowPassFilter 

module BassBoostFilter (

    input  logic signed [15:0] sample_in,
    input logic signed [31:0] low_in,
    output logic signed [31:0] bassBoost_out // Se trunca en el principal, para no perder información

);

    always_comb begin

        // Fórmula para el filtro
        bassBoost_out = {{16{sample_in[15]}}, sample_in} + (low_in >>> 1); // x[n] + k * low[n] (donde k es 1/2 como valor inicial))
    end

endmodule