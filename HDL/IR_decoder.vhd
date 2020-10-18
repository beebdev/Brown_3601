library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity IR_decoder is
	port( Din:	in std_logic;
			btn:	in std_logic;
			mclk:	in std_logic;
			led:	out std_logic_vector(7 downto 0);
			an:	out std_logic_vector(3 downto 0);
			ssg:	out std_logic_vector(7 downto 0));
end IR_decoder;

architecture Behavioral of IR_decoder is
	-- states
	type state_t is (s_ready, s_lead_H, s_lead_L, s_pulse, s_space, s_done);
	signal state : state_t;
	
	-- clk generation
	signal clk1 : std_logic := '0';
	signal clk2 : std_logic := '0';
	signal clk1_cnt : std_logic_vector(15 downto 0) := (others => '0');
	signal clk2_cnt : std_logic_vector(16 downto 0) := (others => '0');
	
	-- timer and bitcount
	signal tcnt : unsigned(7 downto 0) := (others => '0');
	signal bcnt : unsigned(5 downto 0) := (others => '0');
	
	-- Memory
	signal mem_buf : std_logic_vector(15 downto 0) := (others => '0');
	
	-- others
	signal reset : std_logic;
	
	-- Peripherals
	signal ctrl : std_logic_vector(3 downto 0);
	signal dig : std_logic_vector(6 downto 0);
begin
	reset <= btn;
	led <= std_logic_vector(tcnt);
	
	-- clk generate
	process (mclk)
	begin
		if mclk = '1' and mclk'event then
			clk1_cnt <= clk1_cnt + 1;
			clk2_cnt <= clk2_cnt + 1;
			if clk1_cnt = "0001010111111001" then
				clk1_cnt <= (others => '0');
				clk1 <= not clk1;
			end if;
			
			if clk2_cnt = "11000011010100000" then
				clk2_cnt <= (others => '0');
				clk2 <= not clk2;
			end if;
		end if;
	end process;


	-- State transition
	process (reset, clk1)
	begin
		if reset = '1' then
			state <= s_ready;
		elsif clk1 = '1' and clk1'event then
			case state is
				when s_ready =>
					tcnt <= (others => '0');
					bcnt <= (others => '0');
					mem_buf <= (others => '0');
					if Din = '0' then
						state <= s_lead_H;
					else
						state <= s_ready;
					end if;
				when s_lead_H =>
					if Din = '0' then
						state <= s_lead_H;
						tcnt <= tcnt + 1;
					elsif Din = '1' then
						if tcnt >= 79 and tcnt <= 81 then
							state <= s_lead_L;
							tcnt <= (others => '0');
						else
							state <= s_ready;
						end if;
					end if;
				when s_lead_L =>
					if Din = '1' then
						state <= s_lead_L;
						tcnt <= tcnt + 1;
					elsif Din = '0' then
						if tcnt >= 38 and tcnt <= 42 then
							state <= s_pulse;
							tcnt <= (others => '0');
						else
							state <= s_ready;
						end if;
					end if;
				when s_pulse =>
					if Din = '0' then
						state <= s_pulse;
						tcnt <= tcnt + 1;
					elsif Din = '1' then
						if tcnt >= 4 and tcnt <= 6 then
							if bcnt < 32 then
								state <= s_space;
								tcnt <= (others => '0');
								bcnt <= bcnt + 1;
							else
								state <= s_done;
							end if;
						else
							state <= s_ready;
						end if;
					end if;
				when s_space =>
					if Din = '1' then
						state <= s_space;
						tcnt <= tcnt + 1;
					elsif Din = '0' then
						if tcnt >= 3 and tcnt <= 5 then
							state <= s_pulse;
							tcnt <= (others => '0');
							if (bcnt > 8 and bcnt <= 16) or bcnt > 24 then
								mem_buf <= '0' & mem_buf(15 downto 1);
							end if;
						elsif tcnt >= 13 and tcnt <= 15 then
							state <= s_pulse;
							tcnt <= (others => '0');
							if (bcnt > 8 and bcnt <= 16) or bcnt > 24 then
								mem_buf <= '1' & mem_buf(15 downto 1);
							end if;
						else
							state <= s_ready;
						end if;
					end if;
				when s_done =>
					state <= s_done;
			end case;
		end if;
	end process;
	
	-- output
	process(state)
	begin
		--done <= '0';
		case state is
			when s_ready =>
			when s_lead_H =>
			when s_lead_L =>
			when s_pulse =>
			when s_space =>
			when s_done =>
		end case;
	end process;
	
	-- 7-seg controller
	process (clk2)
		variable digit : std_logic_vector(1 downto 0) := (others => '0');
	begin
		if clk2'event and clk2 = '1' then
			digit := digit + 1;
			case digit is
				when "00" =>
					an <= "0111";
					ctrl <= mem_buf(15 downto 12);
				when "01" =>
					an <= "1011";
					ctrl <= mem_buf(11 downto 8);
				when "10" =>
					an <= "1101";
					ctrl <= mem_buf(7 downto 4);
				when "11" =>
					an <= "1110";
					ctrl <= mem_buf(3 downto 0);
				when others =>
					an <= (others => '1');
					ctrl <= (others => '0');
			end case;
		end if;
	end process;

	-- 7-seg display
	with ctrl select
	dig<= "1111001" when "0001",   --1
         "0100100" when "0010",   --2
         "0110000" when "0011",   --3
         "0011001" when "0100",   --4
         "0010010" when "0101",   --5
         "0000010" when "0110",   --6
         "1111000" when "0111",   --7
         "0000000" when "1000",   --8
         "0010000" when "1001",   --9
         "0001000" when "1010",   --A
         "0000011" when "1011",   --b
         "1000110" when "1100",   --C
         "0100001" when "1101",   --d
         "0000110" when "1110",   --E
         "0001110" when "1111",   --F
         "1000000" when others;   --0
	ssg(6 downto 0) <= dig;
	ssg(7) <= '0';
end Behavioral;

