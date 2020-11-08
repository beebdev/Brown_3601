library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity ir_decoder is
	port (reset :	in std_logic;
			mclk :	in std_logic;
			din :		in std_logic;
			IRQ :		out std_logic;
			dout :	out std_logic_vector(15 downto 0));
end ir_decoder;

architecture Behavioral of ir_decoder is
	-- state machine
	type state_t is (s_ready, s_lead_H, s_lead_L, s_pulse, s_space, s_done);
	signal state :	state_t;
	
	-- clk generation
	signal clk : std_logic := '0';
	signal clk_cnt : std_logic_vector(15 downto 0);
	signal period : integer := 5625;
	
	-- counters
	signal tcnt : integer;
	signal bcnt : integer range 0 to 32;
	
	-- memory buffer
	signal mem_buf : std_logic_vector(15 downto 0);
begin
	-- State transition
	process (reset, clk)
	begin
		if reset = '1' then
			state <= ready;
		elsif clk'event and clk = '1' then
			dout <= (others => '0');
			case state is
				when s_ready =>
					tcnt <= 0;
					bcnt <= 0;
					mem_buf <= (others => '0');
					
					if din = '0' then
						state <= s_lead_H;
					else
						state <= s_ready;
					end if;
				when s_lead_H =>
					if din = '0' then
						state <= s_lead_h;
						tcnt <= tcnt + 1;
					elsif din = '1' then
						if tcnt >= 79*period and tcnt <= 81*period then
							state <= s_lead_L;
							tcnt <= (others => '0');
						else
							state <= s_ready;
						end if;
					end if;
				when s_lead_L =>
					if din = '1' then
						state <= s_lead_L;
						tcnt <= tcnt + 1;
					elsif din = '0' then
						if tcnt >= 38*period and tcnt <= 42*period then
							state <= s_pulse;
							tcnt <= (others => '0');
						else
							state <= s_ready;
						end if;
					end if;
				when s_pulse =>
					if din = '0' then
						state <= s_pulse;
						tcnt <= tcnt + 1;
					elsif din = '1' then
						if tcnt >= 4*period and tcnt <= 6*period then
							if bcnt < 32 then
								state <= s_space;
								tcnt <= 0;
								bcnt <= bcnt + 1;
							else
								state <= s_done;
							end if
						else
							state <= s_ready;
						end if;
					end if;
				when s_space =>
					if din = '1' then
						state <= s_space;
						tcnt <= tcnt + 1;
					elsif din = '0' then
						if tcnt >= 3*period and tcnt <= 5*period then
							state <= s_pulse;
							tcnt <= (others => '0');
							if (bcnt > 8 and bcnt <=16) or bcnt > 24 then
								mem_buf <= '0' & mem_buf(15 downto 1);
							end if;
						elsif tcnt >= 13*period and tcnt <= 15*period then
							state <= pulse;
							tcnt <= (others => '0');
							if (bcnt > 8 and bcnt <=16) or bcnt > 24 then
								mem_buf <= '1' & mem_buf(15 downto 1);
							end if;
						else
							state <= s_ready;
						end if;
					end if;
				when s_done =>
					state <= s_done;
					dout <= mem_buf;
			end case;
		end if;
	end process;

end Behavioral;

