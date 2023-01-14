////////////////////////////////////////////////////////////////////////
//	Filename:	ADDRCTLR.v
//	Author:		Geoff Richmond
//	
//	Description:	Configurable SDIO Address Controller Module.
//					The Address width is configurable.
//
////////////////////////////////////////////////////////////////////////




module ADDR_CTLR(	ACTL_StrbN,
					ACTL_RstN,
					ACTL_Data_In,
					ACTL_Inc,
					ACTL_BSel,
					ACTL_CmdN,
				
					ACTL_Addr_Out
				);
			
// Inputs
input 				ACTL_StrbN;		// Address latch strobe
input					ACTL_RstN;		// Active low Reset signal
input [(`DWIDTH-1):0]	ACTL_Data_In;	// Data into ADDR_CTLR
input					ACTL_Inc;		// Increment Address signal
input [(`BSWIDTH-1):0]	ACTL_BSel;		// Bank Select
input					ACTL_CmdN;		// Command control

// Outputs
output [(`AWIDTH-1):0]	ACTL_Addr_Out;	// Address bus out

// Combinational Logic
reg [(`AWIDTH-1):0]		ACTL_Addr_Out;		// Address register


// Clocked Logic

// Control circuitry for Address Controller. If reset 
// is asserted, the address latch is set to 0. The address latch
// is loaded in blocks of the input bus width. The falling edge  
// of the strobe latches the data into the address register
// selected by Bank Select. If IncN is asserted, the address is
// incremented with each falling edge of StrbN.
always @(posedge ACTL_StrbN or negedge ACTL_RstN)
begin
	
	if(!ACTL_RstN) begin
		ACTL_Addr_Out	<= 0;
	end
	
	else begin
		
		// If we are in command mode, then do nothing.
		if(ACTL_CmdN) begin
			
			// Address increment
			if(ACTL_Inc)
				ACTL_Addr_Out	<= ACTL_Addr_Out + 1;

			// Load Address Bank 0-2
			if(ACTL_BSel == 3'b000)
				ACTL_Addr_Out[7:0]	<= ACTL_Data_In;
				
			else if(ACTL_BSel == 3'b001)
				ACTL_Addr_Out[15:8]	<= ACTL_Data_In;
				
			else if(ACTL_BSel == 3'b010)
				ACTL_Addr_Out[16]	<= ACTL_Data_In[0];

		end				
	end
end


endmodule // ADDR_CTLR



