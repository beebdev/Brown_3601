library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity button_buffer is
	port( reset : 	in std_logic;
			mclk :	in std_logic;
			btn :		in std_logic_vector(3 downto 0);
			Dout :	out std_logic_vector(1 downto 0);
			
			ACK :		in std_logic;
			IRQ :		out std_logic );
end button_buffer;

architecture Behavioral of button_buffer is
	type state_t is (s_idle, s_sending, s_sent);
	signal state : state_t;
	
	signal tcnt : integer;
begin
	process (reset, mclk)
	begin
		if reset = '1' then
			state <= s_idle;
		elsif mclk'event and mclk = '1' then
		 case state is
			when s_idle =>
				IRQ <= '0';
				Dout <= "00";
				-- Set IRQ high when btn pressed
				if btn /= "0000" then
					state <= s_sending;
					IRQ <= '1';
					if btn(0) = '1' then
						Dout <= "00";
					elsif btn(1) = '1' then
						Dout <= "01";
					elsif btn(2) = '1' then
						Dout <= "10";
					elsif btn(3) = '1' then
						Dout <= "11";
					else
						state <= s_idle;
					end if;
				end if;
			when s_sending =>
				-- Disable IRQ if tx_buffer ACKs
				if ACK = '1' then
					IRQ <= '0';
					tcnt <= 0;
					state <= s_sent;
				else
					state <= s_sending;
				end if;
			when s_sent =>
				if tcnt = 35000000 then
					state <= s_idle;
				else
					tcnt <= tcnt + 1;
					state <= s_sent;
				end if;
		 end case;
		end if;
	end process;

end Behavioral;

