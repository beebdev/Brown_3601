library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity brown_box is
	port (-- FPGA on board peripherals
			mclk:		in std_logic;
			swt :		in std_logic_vector(1 downto 0);
			btn :		in std_logic_vector(3 downto 0);
			led :		out std_logic_vector(7 downto 0);
			an :		out std_logic_vector(3 downto 0);
			ssg :		out std_logic_vector(7 downto 0);
			
			-- IR input
			IR_din :	in std_logic;
			 
			-- SPI interface
			spi_miso:		in std_logic;
			spi_mosi:		out std_logic;
			spi_sclk:		out std_logic;
			spi_ss:			out std_logic );
end brown_box;

architecture Behavioral of brown_box is
begin


end Behavioral;

