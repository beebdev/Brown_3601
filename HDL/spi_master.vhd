library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity spi_master is
	port( reset :		in std_logic;
			mclk :		in std_logic;
			enable :		in std_logic;
			busy :		out std_logic;
			tx_data :	in std_logic_vector(31 downto 0);
			--rx_data :	out std_logic_vector(31 downto 0);
			
			-- SPI Interface
			spi_miso :	in std_logic;
			spi_mosi :	out std_logic;
			spi_ss	:	out std_logic;
			spi_sclk :	out std_logic );
end spi_master;

architecture Behavioral of spi_master is
	-- state machine
	type state_t is (s_ready, s_execute);
	signal state : state_t;

	-- counters and buffers
	signal sclk : std_logic;
	signal tcnt : std_logic_vector(3 downto 0);
	signal bcnt : std_logic_vector(5 downto 0);
	signal dbuf : std_logic_vector(31 downto 0);
begin
	-- Ext/Int signal connections
	spi_sclk <= sclk;
	--rx_data <= dbuf;
	
	-- SPI master ASM
	process (mclk, reset)
	begin
		if reset = '1' then
			busy <= '0';
			sclk <= '0';
			spi_ss <= '1';
			spi_mosi <= 'Z';
			tcnt <= (others => '0');
			bcnt <= (others => '0');
			dbuf <= (others => '0');
			state <= s_ready;
		elsif mclk'event and mclk = '1' then
			case state is
				when s_ready =>
					-- Output
					busy <= '0';
					sclk <= '0';
					spi_ss <= '1';
					spi_mosi <= 'z';
					
					-- State transition
					if enable = '1' then
						busy <= '1';
						spi_ss <= '0';
						dbuf <= tx_data;
						tcnt <= (others => '0');
						bcnt <= (others => '0');
						state <= s_execute;
					else
						state <= s_ready;
					end if;
				when s_execute =>
					-- Common output
					busy <= '1';
					spi_ss <= '0';
					
					-- tcnt check
					if tcnt = "1111" then
						-- toggle sclk and reset tcnt
						sclk <= not sclk;
						tcnt <= (others => '0');
					elsif tcnt = "0110" then
						tcnt <= tcnt + 1;
						bcnt <= bcnt + 1;
						
						-- Check transaction progress
						if bcnt = "100000" then
							sclk <= '0';
							spi_ss <= '1';
							spi_mosi <= 'Z';
							bcnt <= (others => '0');
							state <= s_ready;
						else
							sclk <= not sclk;
							spi_mosi <= dbuf(0);
							dbuf <= spi_miso & dbuf(31 downto 1);
							state <= s_execute;
						end if;
					else
						tcnt <= tcnt + 1;
						state <= s_execute;
					end if;
			end case;
		end if;
	end process;

end Behavioral;

