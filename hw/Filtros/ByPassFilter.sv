// Este filtro es combinacional porque no necesita recordar ningún dato
// Usa la información del filtro LowPassFilter 

module ByPassFilter (

    input  logic signed [15:0] sample_in,
    output logic signed [31:0] byPass_out // Se trunca en el principal, para no perder información
	

);

	 //logic signed [31:0] byPass_out_internal;

    always_comb begin

        // Fórmula para el filtro
        byPass_out = {{16{sample_in[15]}}, sample_in};
		  //byPass_out = 32'sd0;
		  //byPass_out_internal = 32'sd0;
		  //byPass_out = {{16{sample_in[15]}}, sample_in} >>> 8;
    end
	 
	 //assign byPass_out = byPass_out_internal;

endmodule