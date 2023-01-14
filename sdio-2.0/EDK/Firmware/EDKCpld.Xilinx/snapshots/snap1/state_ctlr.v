////////////////////////////////////////////////////////////////////////
//	Filename:	STATE_CTLR.v
//	Author:		Geoff Richmond
//	
//	Description:	SDIO CPLD Controller Module.
//
////////////////////////////////////////////////////////////////////////

module STATE_CTLR(	SC_IClk,
					SC_PClk,
					SC_ResetN,
					SC_Data_Bus,
					SC_CmdN,
					SC_Fifo_Full,
					SC_StrbN,
					SC_BSel,
					SC_Addr_Inc,
					SC_RdyN,
					SC_DO_OutputEnable
				);
			
// Inputs
input					SC_IClk;			// Instruction Clock
input					SC_PClk;			// Port Clock
input					SC_ResetN;		// Active low Reset signal
input	[(`DWIDTH-1):0]			SC_Data_Bus;		// Data Bus
input 				SC_Fifo_Full;	// FIFO Full
input					SC_CmdN;			// Command/Data control

// Outputs
output 				SC_StrbN;		// Address latch strobe
output [(`BSWIDTH-1):0]	SC_BSel;	// Bank Select
output				SC_Addr_Inc;	// Increment Address signal
output				SC_RdyN;			// Ready status line - active low
output				SC_DO_OutputEnable;		// DO Hi-Z Control

// Status Register.
//		Bit 6		- Address increment control
//		Bit 5		- SPI DO Enable
//		Bit 4		- Addr Ctlr Strb Select
//		Bit 3		- Addr Ctlr Strb Enable
//		Bit 1		- Bank Select 1
//		Bit 0		- Bank Select 0
//
`define	STATBITS		8		// Number of bits in status register - 8 bits
reg [(`DWIDTH-1):0]		SC_Status;

assign					SC_BSel[2]	= SC_Status[2];
assign					SC_BSel[1]	= SC_Status[1];
assign					SC_BSel[0]	= SC_Status[0];

assign					SC_DO_OutputEnable		= SC_Status[5];

// Combinational Logic

wire						SC_StrbClk;
wire						SC_StrbN;
wire [(`BSWIDTH-1):0]	SC_BSel;
wire						SC_Addr_Inc;

// FIFO Controls
assign					SC_RdyN = SC_Fifo_Full;

// Address Controller controls
assign					SC_StrbClk = (SC_Status[4]) ? SC_PClk : SC_IClk;
assign					SC_StrbN = (SC_Status[3]) ? SC_StrbClk : 1'b1;
assign					SC_Addr_Inc = SC_Status[6];


// Clocked Logic

// Control circuitry for State Controller. If reset 
// is asserted, clear the status register. 

always @(posedge SC_IClk or negedge SC_ResetN) begin

	if(!SC_ResetN) begin
		SC_Status <= 0;
	end

	else begin
		// Load the status register
		if(!SC_CmdN) begin
			SC_Status[(`STATBITS-1):0]	<= SC_Data_Bus[(`STATBITS-1):0];
		end
	end

end

endmodule // STATE_CTLR



