/*******************************************************************************
    Verilog netlist generated by IPGEN Lattice Radiant Software (64-bit)
    2022.1.0.52.3
    Soft IP Version: 2.1.0
    2023 02 05 13:29:57
*******************************************************************************/
/*******************************************************************************
    Wrapper Module generated per user settings.
*******************************************************************************/
module ebr8x16 (clk_i, rst_i, clk_en_i, wr_en_i, wr_data_i, addr_i, rd_data_o)/* synthesis syn_black_box syn_declare_black_box=1 */;
    input  clk_i;
    input  rst_i;
    input  clk_en_i;
    input  wr_en_i;
    input  [15:0]  wr_data_i;
    input  [7:0]  addr_i;
    output  [15:0]  rd_data_o;
endmodule