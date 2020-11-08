library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity Speaker is
	Port( reset :		in std_logic;
			mclk :		in std_logic;
			mute :		in std_logic;
			IR_IRQ :		in std_logic;
			btn :			in std_logic_vector(3 downto 0);
			AC0 :			out std_logic);
end Speaker;

architecture Behaviour of Speaker is
	-- state machine
	type state_t is (s_standby, s_idle, s_play);
	signal state : state_t;
	
	signal cclk : std_logic := '0';
	signal cclk_div : integer;
	
	signal dcnt : integer;
	signal ncnt : integer;
	signal bcnt : std_logic_vector(3 downto 0);
	signal tcnt : std_logic_vector(7 downto 0);

	signal sig_AC0 : std_logic;
	signal sig_play : std_logic := '0';
	signal sig_off_music : std_logic := '0';
	signal sig_done : std_logic := '1';
	
	type note_array is array(0 to 7) of std_logic_vector(7 downto 0);
	type duration_array is array(0 to 6) of std_logic_vector(3 downto 0);
	type tone_array is array(0 to 6, 0 to 7) of std_logic_vector(3 downto 0);
	signal note_mem : note_array;
	signal duration_mem : duration_array;
	signal tone_mem : tone_array;

begin
	AC0 <= sig_AC0 and (not mute);
	note_mem <= (X"BF", X"AA", X"97", X"8F", X"7F", X"71", X"65", X"5F");
	duration_mem <= (X"6", X"6", X"2", X"2", X"2", X"2", X"2");
	tone_mem <= ((X"0", X"0", X"4", X"4", X"5", X"5", X"4", X"0"),
					 (X"3", X"3", X"2", X"2", X"1", X"1", X"0", X"0"),
					 (X"0", X"1", X"0", X"0", X"0", X"0", X"0", X"0"),
					 (X"1", X"0", X"1", X"0", X"0", X"0", X"0", X"0"),
					 (X"0", X"1", X"2", X"0", X"0", X"0", X"0", X"0"),
					 (X"2", X"1", X"0", X"0", X"0", X"0", X"0", X"0"),
					 (X"0", X"2", X"0", X"0", X"0", X"0", X"0", X"0"));

	process (mclk)
	begin
		if mclk'event and mclk = '1' then
			cclk_div <= cclk_div + 1;
			if cclk_div = 1000 then
				cclk_div <= 0;
				cclk <= not cclk;
			end if;
		end if;
	end process;

	-- State machine
	process (cclk)
		variable var_sel : integer range 0 to 6;
		variable var_off : std_logic;
	begin
		if cclk'event and cclk = '1' then
			if reset = '1' then
				if sig_off_music = '1' then
					sig_done <= '0';
					sig_off_music <= '0';
					var_sel := 1;
					var_off := '0';
					dcnt <= 0;
					ncnt <= 0;
					tcnt <= (others =>'0');
					bcnt <= (others => '0');
					state <= s_play;
				elsif sig_done = '1' then
					state <= s_standby;
				end if;
			end if;

			case state is
				when s_standby =>
					var_sel := 0;
					var_off := '0';
					dcnt <= 0;
					ncnt <= 0;
					tcnt <= (others =>'0');
					bcnt <= (others => '0');
					if reset = '1' then
						state <= s_standby;
					else
						state <= s_play;
					end if;
				when s_idle =>
					sig_off_music <= '1';
					sig_AC0 <= '0';
					var_off := '0';
					dcnt <= 0;
					ncnt <= 0;
					tcnt <= (others =>'0');
					bcnt <= (others => '0');
					-- Check button press
					if btn /= "0000" then
						state <= s_play;
						if btn(0) = '1' then
							var_sel := 5;
						elsif btn(1) = '1' then
							var_sel := 4;
						elsif btn(2) = '1' then
							var_sel := 3;
						elsif btn(3) = '1' then
							var_sel := 2;
						else
							state <= s_idle;
						end if;
					-- Check IR signal
					elsif IR_IRQ = '1' then
						var_sel := 6;
						state <= s_play;
					end if;
				when s_play =>
					if dcnt = 5000 then
						ncnt <= ncnt + 1;
						if ncnt = 5 then
							if bcnt = duration_mem(var_sel) then
								sig_done <= '1';
								state <= s_idle;
							else
								var_off := '0';
								bcnt <= bcnt + 1;
								dcnt <= 0;
								ncnt <= 0;
								state <= s_play;
							end if;
						elsif ncnt = 4 then
							var_off := '1';
						end if;
					else
						if tcnt = note_mem(conv_integer(tone_mem(var_sel, conv_integer(bcnt))))
							and var_off <= '0' then
							sig_AC0 <= not sig_AC0;
							tcnt <= (others => '0');
						else
							tcnt <= tcnt + 1;
						end if;
						dcnt <= dcnt + 1;
					end if;
			end case;
		end if;
	end process;
end Behaviour;

