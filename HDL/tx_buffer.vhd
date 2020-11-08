library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity tx_buffer is
	port( reset :		in std_logic;
			mclk :		in std_logic;
			IRQ :			in std_logic;
			IR_in :		in std_logic_vector(15 downto 0);
			vol_in :		in std_logic_vector(15 downto 0);
			ACK :			out std_logic;

			-- SPI master interface
			spi_busy :	in std_logic;
			spi_send :	out std_logic;
			tx_data :	out std_logic_vector(31 downto 0));
end tx_buffer;

architecture Behavioral of tx_buffer is
	signal vol_mem : std_logic_vector(15 downto 0);
	signal IR_mem : std_logic_vector(15 downto 0);
	
	type state_t is (s_check, s_send);
	signal state : state_t;
begin
	
	vol_tx : process(reset, cclk)
	begin
		if reset = '1' then
			vol_mem <= (others => '0');
			IR_mem <= (others => '0');
			state <= s_check;
		elsif cclk'event and cclk = '1' then
			case state is
				when s_check =>
					ACK <= '0';
					if IRQ = '1' then
						IR_mem <= IR_in;
						vol_mem <= vol_in;
						ACK <= '1'
						state <= s_send;
					elsif
						IR_mem <= (others => '0');
						vol_mem <= IR_in;
						state <= s_send;
					else
						state <= s_check;
					end if;
				when s_send =>
					ACK <= '0';
					spi_send <= '1';
					tx_data <= IR_mem & vol_mem;
					if spi_busy = '1' then
						state <= s_send;
					else
						state <= s_check;
					end if;
			end case;
		end if;
	end process;

end Behavioral;

