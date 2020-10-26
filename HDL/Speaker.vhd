
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity Speaker is
    Port (
        mclk    : in std_logic;
        btn     : in std_logic_vector(3 downto 0);
		  AC0		 : out std_logic);
end Speaker;

ARCHITECTURE Behaviour OF Speaker IS

	------------------------------------------------------------------------
	-- Component Declarations
	------------------------------------------------------------------------
	
	
	------------------------------------------------------------------------
	-- State Declarations
	------------------------------------------------------------------------

	TYPE State_type IS (standby, playing0, playing1, playing2, playing3, waiting);
	Signal y, y_next : State_type;
	
	------------------------------------------------------------------------
	-- Signal Declarations
	------------------------------------------------------------------------

	signal clkdiv  : std_logic_vector(25 downto 0);
	signal cclk    : std_logic;
	signal cclk_en : std_logic;

	------------------------------------------------------------------------
	-- Module Implementation
	------------------------------------------------------------------------

	begin

	statetable: PROCESS (y, cclk, btn(0), btn(1), btn(2), btn(3))
	BEGIN
		CASE y IS
			WHEN standby =>
				IF btn(0) = '1' THEN
					y_next <= playing0;	
				ELSIF btn(1) = '1' THEN
					y_next <= playing1;
				ELSIF btn(2) = '1' THEN
					y_next <= playing2;
				ELSIF btn(3) = '1' THEN
					y_next <= playing3;
				ELSE 
					y_next <= standby;
				END IF;
			WHEN playing0 => 
				IF cclk = '1' THEN
					y_next <= waiting;
				ELSE 
					y_next <= playing0;
				END IF;
			WHEN playing1 => 
				IF cclk = '1' THEN
					y_next <= waiting;
				ELSE 
					y_next <= playing1;
				END IF;		
			WHEN playing2 => 
				IF cclk = '1' THEN
					y_next <= waiting;
				ELSE 
					y_next <= playing2;
				END IF;		
			WHEN playing3 => 
				IF cclk = '1' THEN
					y_next <= waiting;
				ELSE 
					y_next <= playing3;
				END IF;
			WHEN waiting => --wait for button to be released
				IF btn(0) = '1' THEN
					y_next <= waiting;	
				ELSIF btn(1) = '1' THEN
					y_next <= waiting;
				ELSIF btn(2) = '1' THEN
					y_next <= waiting;
				ELSIF btn(3) = '1' THEN
					y_next <= waiting;
				ELSE 
					y_next <= standby;
				END IF;
		END CASE;
	END PROCESS;
	
	controlsignals: PROCESS (y)
	BEGIN
		CASE y IS
			WHEN standby =>
				cclk_en <= '0';
			WHEN playing0 => 
				cclk_en <= '1';
			
			WHEN playing1 => 
				cclk_en <= '1';
			
			WHEN playing2 => 
				cclk_en <= '1';
			
			WHEN playing3 => 
				cclk_en <= '1';
				
			WHEN waiting =>
				cclk_en <= '0';
		END CASE;
	END PROCESS;
	
	fsmflipflops: PROCESS (mclk)
	BEGIN
		IF mclk'event AND mclk = '1' THEN
			y <= y_next;
		END IF;
	END PROCESS;

	-- Divide the master clock (100Mhz) down to a lower frequency.
	PROCESS (mclk)
	BEGIN
		IF mclk = '1' AND mclk'Event THEN
			IF cclk_en = '1' THEN 
				clkdiv <= clkdiv + 1;
			ELSE
				clkdiv <= "00000000000000000000000000";
			END IF;
		END IF;
	END PROCESS;

	cclk <= clkdiv(25);
	AC0 <= clkdiv(18);


END Behaviour;

