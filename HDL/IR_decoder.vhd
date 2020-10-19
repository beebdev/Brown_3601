library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity ir_decoder is
	port (reset :	in std_logic;
			mclk :	in std_logic;
			din :		in std_logic;
			dout :	out std_logic_vector(15 downto 0));
end ir_decoder;

architecture Behavioral of ir_decoder is
	type state_t is (s_ready, s_lead_H, s_lead_L, s_pulse, s_space, s_done);
	signal state :	state_t;
	
	-- clk generation
	signal clk : std_logic := '0';
	signal clk_cnt : std_logic_vector(15 downto 0);
	
	-- counters
	signal tcnt : std_logic_vector(7 downto 0);
	signal bcnt : std_logic_vector(5 downto 0);
	
	-- memory buffer
	signal mem_buf : std_logic_vector(15 downto 0);
begin
	-- CLK generation
	process (mclk)
	begin
		if mclk'event and mclk = '1' then
			clk_cnt <= clk_cnt + 1;
			if clk_cnt = "0001010111111001" then
				clk_cnt <= (others => '0');
				clk <= not clk;
			end if;
		end if;
	end process;
	
	-- State transition
	process (reset, clk)
	begin
		if reset = '1' then
			state <= ready;
		elsif clk'event and clk = '1' then
			dout <= (others => '0');
			case state is
				when s_ready =>
					tcnt <= (others => '0');
					bcnt <= (others => '0');
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
						if tcnt >= 79 and tcnt <= 81 then
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
						if tcnt >= 38 and tcnt <= 42 then
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
						if tcnt >= 4 and tcnt <=6 then
							if bcnt < 32 then
								state <= s_space;
								tcnt <= (others => '0');
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
						if tcnt >= 3 and tcnt <= 5 then
							state <= s_pulse;
							tcnt <= (others => '0');
							if (bcnt > 8 and bcnt <=16) or bcnt > 24 then
								mem_buf <= '0' & mem_buf(15 downto 1);
							end if;
						elsif tcnt >= 13 and tcnt <=15 then
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

