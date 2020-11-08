library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;

entity ADC is    
	Port( mclk :		in std_logic;
			ADC_miso :	in std_logic;
			ADC_mosi :	out std_logic;
			ADC_ss :		out std_logic;
			ADC_sclk :	out std_logic;
			Dout :		out std_logic_vector(7 downto 0));
end ADC;

architecture Behavioral of ADC is

	type state_t is (s_start, s_sending, s_receiving);
	signal state : state_t;
	
	signal cclk : std_logic := '0';
	signal cclk_div : integer range 0 to 1000001;

	signal scnt: integer;
	
	signal start_cnfg : std_logic_vector(4 downto 0);
	signal rx_buf : std_logic_vector(9 downto 0);
	signal val : std_logic_vector(9 downto 0);
begin
	ADC_sclk <= cclk;
	Dout <= val(9 downto 2);

	slave_clock: process(mclk)
	begin
		if mclk'event and mclk = '1' then 
			if cclk_div < 1000000 then 
				cclk_div <= cclk_div + 1;
			else
				cclk <= not cclk;
				cclk_div <= 0;
			end if;
		end if;            
	end process;



	process(cclk)
	begin
		if cclk'event and cclk = '0' then      
			case state is
				when s_start =>
					ADC_ss <= '1';
					ADC_MOSI <= '0';
					start_cnfg <= "11000";
					rx_buf <= (others => '0');
					scnt <= 4;
					state <= s_sending;
				-- Init config
				when s_sending =>
					ADC_ss <= '0';
					start_cnfg <= start_cnfg(3 downto 0) & start_cnfg(4); 
					ADC_MOSI <= start_cnfg(4);
					scnt <= scnt - 1;
					if scnt = 0 then 
						ADC_MOSI <= '0';
						scnt <= 12;
						state <= s_receiving;
					end if;
				-- Read data
				when s_receiving=>
					ADC_ss <= '0';
					if scnt = 0 then
						ADC_MOSI <= '0';
						val <= rx_buf;
						state <= s_start;
					elsif scnt < 11 then 
						rx_buf <=  rx_buf(8 downto 0) & ADC_miso;
						scnt <= scnt - 1;
					else
						scnt <= scnt - 1;
					end if;
				when others =>    
					state <= s_start;           
			end case;
		end if;  
	end process;

end Behavioral;
