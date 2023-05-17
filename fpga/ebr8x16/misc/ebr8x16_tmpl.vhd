component ebr8x16 is
    port(
        clk_i: in std_logic;
        rst_i: in std_logic;
        clk_en_i: in std_logic;
        wr_en_i: in std_logic;
        wr_data_i: in std_logic_vector(15 downto 0);
        addr_i: in std_logic_vector(7 downto 0);
        rd_data_o: out std_logic_vector(15 downto 0)
    );
end component;

__: ebr8x16 port map(
    clk_i=>,
    rst_i=>,
    clk_en_i=>,
    wr_en_i=>,
    wr_data_i=>,
    addr_i=>,
    rd_data_o=>
);
