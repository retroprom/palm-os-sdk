////////////////////////////////////////////////////////////////////////
//	Filename:	FIFO.v
//	Author:		Geoff Richmond
//	
//	Description:	Configurable Parallel-In, Parallel-Out FIFO
//					The FIFO depth (and width) are configurable.
//
////////////////////////////////////////////////////////////////////////



module FIFO(	FIFO_Clk,
				FIFO_RstN,
				FIFO_Data_In,
				FIFO_WriteN,
				FIFO_RcvrLoadingN,
				FIFO_RcvrRdyN,
				FIFO_CmdN,
				
				FIFO_Data_Out,
				FIFO_Full,
				FIFO_DataRdyN
			);
			
// Inputs
input					FIFO_Clk;		// CLK signal
input					FIFO_RstN;		// Active low Reset signal
input [(`FWIDTH-1):0]	FIFO_Data_In;	// Data into FIFO
input					FIFO_WriteN;		// Control signal - write
input					FIFO_RcvrLoadingN;		// Receiver is loading data
input					FIFO_RcvrRdyN;		// Receiver is ready for data
input					FIFO_CmdN;		// Command control

// Outputs
output [(`FWIDTH-1):0]	FIFO_Data_Out;	// FIFO Data out
output					FIFO_Full;	// FIFO Full signal
output					FIFO_DataRdyN;	// Data is ready in output buffer, in response to FIFO_RcvrRdyN

// Combinational Logic
reg [(`FWIDTH-1):0]	FIFO_Data_Out;		// Output buffer
reg						FIFO_FullReg;
reg						FIFO_DataRdyN;

reg						FIFO_RcvrRdy;
reg						FIFO_RcvrLoading;

wire						FIFO_Full;

assign					FIFO_Full = FIFO_FullReg | FIFO_RcvrLoading;

// Clocked Logic

// Control circuitry for FIFO. If reset or clr signal is asserted,
// all the counters are set to 0. If write, the write counter is 
// incremented. If read, the read counter is incremented. If both,
// both counters are incremented. The number of items in the FIFO
// is indicated by fcounter.
always @(posedge FIFO_Clk or negedge FIFO_RstN)
begin
	
	if(!FIFO_RstN) begin
		FIFO_Data_Out	= 8'b11111111;
		FIFO_FullReg	= 1'b0;
		FIFO_DataRdyN	= 1'b1;
	end
	
	else begin
					
		// If we are in command mode, do nothing
		// If WriteN active, write data to buffer
		if( FIFO_CmdN && ~FIFO_WriteN ) begin
			FIFO_Data_Out	= FIFO_Data_In;
			FIFO_FullReg	= 1'b1;
		end

		// Handshaking with SPI Shift Register
		if( FIFO_RcvrRdy && FIFO_FullReg ) begin
			FIFO_DataRdyN	= 1'b0;
		end
		
		if( ~FIFO_RcvrRdy && ~FIFO_DataRdyN ) begin
			FIFO_FullReg	= 1'b0;
		end

		if( ~FIFO_FullReg ) begin
			FIFO_DataRdyN	= 1'b1;
		end
	end
end
	
always @(negedge FIFO_Clk or negedge FIFO_RstN)
begin
	if(!FIFO_RstN) begin
		FIFO_RcvrRdy	<= 0;
	end
	
	else begin
		// Handshaking with SPI Shift Register
		FIFO_RcvrRdy	<= ~FIFO_RcvrRdyN;
		FIFO_RcvrLoading	<= ~FIFO_RcvrLoadingN;
	end
end		

endmodule	// FIFO	



