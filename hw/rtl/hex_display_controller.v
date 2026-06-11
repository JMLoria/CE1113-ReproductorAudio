//=================================================================
// hex_display_controller.v
//
// Módulo de hardware personalizado para REQ-04 / REQ-10
// Controla los 6 displays de 7 segmentos de la DE1-SoC vía bus Avalon-MM
//
//
// Mapa de registros (slave Avalon-MM, 32 bits):
//   0x00  CONTROL   R/W   bit 0: enable, bit 1: modo
//                         (modo 0 = MM:SS, modo 1 = hex)
//   0x04  STATUS    R     bit 0: ready (siempre 1)
//   0x08  DATA      R/W   dato a mostrar
//   0x0C  -         -     reservado
//
// Modo 0 (tiempo):
//   DATA = segundos totales (0..3599)
//   HEX3..HEX0 muestra MM:SS, HEX5/HEX4 apagados
//
// Modo 1 (hex):
//   DATA = número hexadecimal de 24 bits
//   HEX5..HEX0 muestra los 6 dígitos hex
//=================================================================

module hex_display_controller (
    // Avalon-MM slave interface
    input  wire        clk,
    input  wire        reset_n,
    input  wire [1:0]  avs_address,       // 4 registros => 2 bits
    input  wire        avs_read,
    input  wire        avs_write,
    input  wire [31:0] avs_writedata,
    output reg  [31:0] avs_readdata,

    // Salidas físicas a los 6 displays (activo bajo en DE1-SoC)
    output wire [6:0]  hex0,
    output wire [6:0]  hex1,
    output wire [6:0]  hex2,
    output wire [6:0]  hex3,
    output wire [6:0]  hex4,
    output wire [6:0]  hex5
);

    //-----------------------------------------------------------
    // Registros internos
    //-----------------------------------------------------------
    reg [31:0] control_reg;
    reg [31:0] data_reg;
    wire [31:0] status_reg;

    // El bit 0 es "ready", siempre 1 en esta versión
    assign status_reg = 32'h0000_0001;

    // Aliases legibles
    wire enable      = control_reg[0];
    wire mode_select = control_reg[1];  // 0=tiempo, 1=hex

    //-----------------------------------------------------------
    // Avalon-MM: escritura
    //-----------------------------------------------------------
    always @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            control_reg <= 32'h0;
            data_reg    <= 32'h0;
        end else if (avs_write) begin
            case (avs_address)
                2'b00: control_reg <= avs_writedata;
                2'b10: data_reg    <= avs_writedata;
                default: ; // STATUS es read-only, RESERVED ignora
            endcase
        end
    end

    //-----------------------------------------------------------
    // Avalon-MM: lectura
    //-----------------------------------------------------------
    always @(*) begin
        case (avs_address)
            2'b00:   avs_readdata = control_reg;
            2'b01:   avs_readdata = status_reg;
            2'b10:   avs_readdata = data_reg;
            default: avs_readdata = 32'h0;
        endcase
    end

    //-----------------------------------------------------------
    // Cálculo de los 6 dígitos a mostrar (4 bits cada uno)
    // Se calculan según el modo seleccionado
    //-----------------------------------------------------------
    reg [3:0] digit0, digit1, digit2, digit3, digit4, digit5;
    reg       blank4, blank5;  // los dos displays superiores apagados en modo tiempo

    // Cálculo de MM:SS desde data_reg (cuando modo=0)
    wire [15:0] total_seconds = data_reg[15:0];
    wire [7:0]  minutes = (total_seconds / 60) % 100;  // limitar a 99 minutos
    wire [7:0]  seconds = total_seconds % 60;

    // BCD de minutos y segundos
    wire [3:0] min_tens  = minutes / 10;
    wire [3:0] min_units = minutes % 10;
    wire [3:0] sec_tens  = seconds / 10;
    wire [3:0] sec_units = seconds % 10;

    always @(*) begin
        if (mode_select == 1'b0) begin
            // Modo tiempo MM:SS en HEX3..HEX0
            digit0 = sec_units;
            digit1 = sec_tens;
            digit2 = min_units;
            digit3 = min_tens;
            digit4 = 4'h0;
            digit5 = 4'h0;
            blank4 = 1'b1;  // apagar HEX4
            blank5 = 1'b1;  // apagar HEX5
        end else begin
            // Modo hex en HEX5..HEX0
            digit0 = data_reg[3:0];
            digit1 = data_reg[7:4];
            digit2 = data_reg[11:8];
            digit3 = data_reg[15:12];
            digit4 = data_reg[19:16];
            digit5 = data_reg[23:20];
            blank4 = 1'b0;
            blank5 = 1'b0;
        end
    end

    //-----------------------------------------------------------
    // Instanciar 6 decodificadores hex -> 7-segmentos
    //-----------------------------------------------------------
    wire [6:0] seg0, seg1, seg2, seg3, seg4, seg5;

    seven_seg_decoder dec0 (.bin(digit0), .seg(seg0));
    seven_seg_decoder dec1 (.bin(digit1), .seg(seg1));
    seven_seg_decoder dec2 (.bin(digit2), .seg(seg2));
    seven_seg_decoder dec3 (.bin(digit3), .seg(seg3));
    seven_seg_decoder dec4 (.bin(digit4), .seg(seg4));
    seven_seg_decoder dec5 (.bin(digit5), .seg(seg5));

    //-----------------------------------------------------------
    // Salida: si enable=0, todos apagados; si blank=1, ese apagado
    //-----------------------------------------------------------
    // 7'h7F = todos los segmentos apagados (activo bajo)
    assign hex0 = enable ? seg0 : 7'h7F;
    assign hex1 = enable ? seg1 : 7'h7F;
    assign hex2 = enable ? seg2 : 7'h7F;
    assign hex3 = enable ? seg3 : 7'h7F;
    assign hex4 = (enable & ~blank4) ? seg4 : 7'h7F;
    assign hex5 = (enable & ~blank5) ? seg5 : 7'h7F;

endmodule


//=================================================================
// seven_seg_decoder.v (sub-módulo)
// Decodificador hexadecimal (4 bits) a 7 segmentos.
// Salida activo bajo (convención de la DE1-SoC).
//
// Mapeo de segmentos:
//        --a--
//       |     |
//       f     b
//       |     |
//        --g--
//       |     |
//       e     c
//       |     |
//        --d--
//
// seg = {g, f, e, d, c, b, a}  (bit 6 = g, bit 0 = a)
//=================================================================

module seven_seg_decoder (
    input  wire [3:0] bin,
    output reg  [6:0] seg
);
    always @(*) begin
        case (bin)
            4'h0: seg = 7'b1000000; // 0
            4'h1: seg = 7'b1111001; // 1
            4'h2: seg = 7'b0100100; // 2
            4'h3: seg = 7'b0110000; // 3
            4'h4: seg = 7'b0011001; // 4
            4'h5: seg = 7'b0010010; // 5
            4'h6: seg = 7'b0000010; // 6
            4'h7: seg = 7'b1111000; // 7
            4'h8: seg = 7'b0000000; // 8
            4'h9: seg = 7'b0010000; // 9
            4'hA: seg = 7'b0001000; // A
            4'hB: seg = 7'b0000011; // b
            4'hC: seg = 7'b1000110; // C
            4'hD: seg = 7'b0100001; // d
            4'hE: seg = 7'b0000110; // E
            4'hF: seg = 7'b0001110; // F
            default: seg = 7'b1111111; // todos apagados
        endcase
    end
endmodule