library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity brown_box is
	port (-- FPGA on board peripherals
			mclk:		in std_logic;
			btn :		in std_logic_vector(1 downto 0);
			swt :		in std_logic_vector(7 downto 0);
			led :		out std_logic_vector(7 downto 0);
			an :		out std_logic_vector(3 downto 0);
			ssg :		out std_logic_vector(7 downto 0);
			
			-- Off board inputs
			IR_din :	in std_logic;
			 
			-- Off board outputs
			spi_miso:		in std_logic;
			spi_mosi:		out std_logic;
			spi_sclk:		out std_logic;
			spi_ss:			out std_logic );
end brown_box;

architecture Behavioral of brown_box is
	component spi_master is
	port (clk :			in std_logic;
			reset :		in std_logic;
			enable :		in std_logic;
			busy :		out std_logic;
			tx_data :	in std_logic_vector(31 downto 0);
			rx_data :	out std_logic_vector(31 downto 0);
			
			-- SPI Interface
			spi_miso :	in std_logic;
			spi_mosi :	out std_logic;
			spi_ss	:	out std_logic;
			spi_sclk :	out std_logic );
	end component;
	
	component ir_decoder is
	port (reset :	in std_logic;
			mclk :	in std_logic;
			din :		in std_logic;
			dout :	out std_logic_vector(15 downto 0));
	end component;
begin


end Behavioral;

