module AudioFilterControlAvalon (
    input  logic        clk,
    input  logic        reset,

    // Interfaz Avalon-MM Slave
    input  logic avs_s0_chipselect,
    input  logic avs_s0_write,
    input  logic avs_s0_read,
    input  logic [1:0] avs_s0_address,
    input  logic [31:0] avs_s0_writedata,
    output logic [31:0] avs_s0_readdata,

    // Salida hacia AudioFilter.sv
    output logic [1:0] filter_sel
);

    logic [1:0] filter_sel_reg;

    // Registro de control
    always_ff @(posedge clk or posedge reset) begin
        if (reset) begin
            filter_sel_reg <= 2'b00; // bypass por defecto
        end else begin
            if (avs_s0_chipselect && avs_s0_write) begin
                case (avs_s0_address)
                    2'd0: filter_sel_reg <= avs_s0_writedata[1:0];
                    default: filter_sel_reg <= filter_sel_reg;
                endcase
            end
        end
    end

    // Lectura desde Nios II
    always_comb begin
        case (avs_s0_address)
            2'd0: avs_s0_readdata = {30'd0, filter_sel_reg}; // control register
            2'd1: avs_s0_readdata = {30'd0, filter_sel_reg}; // status register simple
            2'd2: avs_s0_readdata = 32'hA0F10001; // ID/debug
            default: avs_s0_readdata = 32'd0;
        endcase
    end

    assign filter_sel = filter_sel_reg;

endmodule