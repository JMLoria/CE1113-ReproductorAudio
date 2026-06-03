// Este filtro es combinacional porque no necesita recordar ningún dato
// Usa la información del filtro LowPassFilter 

module BassBoostFilter (

    input  logic signed [15:0] sample_in,
    input logic signed [31:0] low_in,
    output logic signed [31:0] bassBoost_out // Se trunca en el principal, para no perder información

);

	 logic signed [31:0] sample_ext;
    logic signed [31:0] low_half;

    always_comb begin

        
		  
        // Extender sample_in de 16 a 32 bits manteniendo el signo
        sample_ext = {{16{sample_in[15]}}, sample_in};

        // Dividir low_in entre 2 manteniendo el signo
        low_half = low_in >>> 1;

        // Fórmula para el filtro
        bassBoost_out = sample_ext + low_half;
		  
    end

endmodule
