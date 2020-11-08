library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity periph_controller is
	port(	DIN :			in std_logic_vector(2 downto 0);
			SPK_play :	out std_logic;
			SPK_sel :	out std_logic_vector(2 downto 0));
end periph_controller;

architecture Behavioral of periph_controller is
	type state_t is (s_idle, s_active);
	signal state : state_t;
begin

end Behavioral;

