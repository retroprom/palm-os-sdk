////////////////////////////////////////////////////////////////////////
//	Filename:	STATE_CTLR.v
//	Author:		Geoff Richmond
//	
//	Description:	SDIO CPLD Controller Module.
//
////////////////////////////////////////////////////////////////////////

`define	DWIDTH		8		// Width of the Data In Bus to load ADDR
`define	BSWIDTH		3		// Bank Select Width (bits necessary to 
									// select each address controller register)

module STATE_CTLR(	SC_IClk,
					SC_PClk,
					SC_ResetN,
					SC_Data_Bus,
					SC_StrbN,
					SC_BSel,
					SC_Addr_Inc
				);
			
// Inputs
input					SC_IClk;			// Instruction Clock
input					SC_PClk;			// Port Clock
input					SC_ResetN;		// Active low Reset signal
input	[(`DWIDTH-1):0]			SC_Data_Bus;		// Data Bus

// Outputs
output 				SC_StrbN;		// Address latch strobe
output [(`BSWIDTH-1):0]	SC_BSel;	// Bank Select
output				SC_Addr_Inc;	// Increment Address signal

// Status Register.
//		Bit 6		- Address increment control
//		Bit 4		- Addr Ctlr Strb Select
//		Bit 3		- Addr Ctlr Strb Enable
//		Bit 2		- Bank Select 2
//		Bit 1		- Bank Select 1
//		Bit 0		- Bank Select 0
//
`define	STATBITS		8		// Number of bits in status register - 8 bits
reg [(`DWIDTH-1):0]	SC_Status;
reg						SC_StateLoadN;			// State register load control

assign					SC_BSel[2]	= SC_Status[2];
assign					SC_BSel[1]	= SC_Status[1];
assign					SC_BSel[0]	= SC_Status[0];


// Combinational Logic

reg						SC_StrbN;
wire [(`BSWIDTH-1):0]	SC_BSel;
wire						SC_Addr_Inc;

// Address Controller controls
assign					SC_Addr_Inc = SC_Status[6];


// Clocked Logic

// Control circuitry for State Controller. If reset 
// is asserted, clear the status register. 

always @(negedge SC_PClk or negedge SC_ResetN) begin

	if(!SC_ResetN) begin
		SC_Status		<= 0;
		SC_StateLoadN	<= 0;
	end

	else begin
		if( ~SC_StateLoadN ) begin
			// Load the status register
			SC_Status[(`STATBITS-1):0]	<= SC_Data_Bus[(`STATBITS-1):0];
			SC_StrbN	<= 1;
			SC_StateLoadN	<= 1;
		end
		else
		begin
			// Strobe the Address Controller
			SC_StrbN	<= 0;
			SC_StateLoadN	<= 0;
		end
	end

end

endmodule // STATE_CTLR



