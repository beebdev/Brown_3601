----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    19:29:55 10/22/2020 
-- Design Name: 
-- Module Name:    ADC - Behavioral 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity ADC is    
	Port ( MISO : in STD_LOGIC;
      CLK : in STD_LOGIC;
      LED : out std_logic_vector(7 downto 0);
		MOSI : out STD_LOGIC;
		CS : out STD_LOGIC;
      SCLK : out STD_LOGIC);
end ADC;

architecture Behavioral of ADC is

signal slave_counter : integer range 0 to 1000001;
signal newClock : std_logic := '0';
signal start_signal : std_logic_vector(4 downto 0);
signal received : std_logic_vector(9 downto 0);
signal val : std_logic_vector(9 downto 0);
signal shift_counter: integer;
type state_type is (start,sending,receiving);  --type of state machine.
signal state : state_type;
begin

slave_clock: process(clk, newClock)
begin
	if rising_edge(clk) then 
		if slave_counter < 1000000 then 
			slave_counter <= slave_counter + 1;
      else
         newClock <= not newClock;
         slave_counter <= 0;
      end if;
   end if;            
end process;

SCLK <= newClock;


SPI_state: process(newClock)
begin
   if newClock'event and newClock = '0' then      
      case state is   
         when start =>
            CS <= '1';
            MOSI <= '0';
				start_signal <= "11000";
				received <= "0000000000";
				shift_counter <= 4;
				state <= sending;
         when sending => -- Send init bits. 
            CS <= '0';
            start_signal <= start_signal(3 downto 0) & start_signal(4); 
            MOSI <= start_signal(4);
            shift_counter <= shift_counter - 1;
            if shift_counter = 0 then 
					MOSI <= '0';
               shift_counter <= 12;
               state <= receiving;
            end if;  
         when receiving=>
            CS <= '0';              -- Read
            if shift_counter = 0 then
               MOSI <= '0';
					val <= received;
               state <= start;
            elsif shift_counter < 11 then 
               received <=  received(8 downto 0) & MISO;
               shift_counter <= shift_counter - 1;
            else
               shift_counter <= shift_counter - 1;
            end if;
         when others =>    
            state <= start;           
      end case;
   end if;  
end process;

LED <= val(9 downto 2);

end Behavioral;

