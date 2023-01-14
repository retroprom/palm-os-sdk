////////////////////////////////////////////////////////////////////////
//	Filename:	SPI_SHIFT_OUT.v
//	
//	Description:	Configurable Parallel Load Shift Register.
//
////////////////////////////////////////////////////////////////////////
// Functionality: If load is asserted, performs parallel load of shift register
//					on the rising edge of sclk. When load is de-asserted,
//					shifts data out, most-significant bit first. The 
//					least-significant bit is loaded with logic high
//					state. When the shift register is empty, the
//					empty output is asserted.
////////////////////////////////////////////////////////////////////////

`define	SDEPTH		8		// Depth of the SPI shift register (bits)
`define	CWIDTH		4		// Width of SPI shift counter, 2^CWIDTH = SDEPTH

module SPI_SHIFT_OUT( 	SPI_Clk,
						SPI_ResetN,
						SPI_Data_RdyN,
						SPI_Data_In,
						SPI_Sdo,
						SPI_RdyN,
						SPI_DataAck
						);

// Inputs
input					SPI_Clk;
input					SPI_ResetN;
input					SPI_Data_RdyN;
input [(`SDEPTH-1):0]	SPI_Data_In;

//Outputs
output 					SPI_Sdo;
output 					SPI_RdyN;
output					SPI_DataAck;

// Sequential Logic
reg [(`SDEPTH-1):0]		SPI_shift_reg;
reg [(`CWIDTH-1):0]		SPI_counter;

reg							SPI_DataAck;

// Combinational Logic

wire			SPI_Sdo	= SPI_shift_reg[(`SDEPTH-1)];

wire			SPI_WriteInhibit;
wire			SPI_LoadN;

// The WriteInhibit signal prevents the parallel input to the shift register
// from changing for 1 1/2 clock cycles before the load.
assign		SPI_WriteInhibit	= (~SPI_counter[3] & ~SPI_counter[2] & ~SPI_counter[1]);

// Shift register parallel loads are performed on falling edge of SCLK
// when counter == 4'b0000
assign		SPI_LoadN	= |SPI_counter;

assign		SPI_RdyN		= (~SPI_DataAck) | (SPI_WriteInhibit);

always @ (negedge SPI_Clk or negedge SPI_ResetN )
begin
	if(~SPI_ResetN) begin
		SPI_shift_reg	<= 8'b11111111;
		SPI_DataAck		<= 1;
	end
	
	else begin
		if ((~SPI_LoadN) & (~SPI_DataAck) )
		begin
			// If there is data ready and the shift register is empty,
			// do a load
			SPI_shift_reg[(`SDEPTH-1):0] <= SPI_Data_In[(`SDEPTH-1):0];
		end
		else
		begin
			// Shift bitstream_in into shift[0]
			SPI_shift_reg[(`SDEPTH-1):1] <= SPI_shift_reg[(`SDEPTH-2):0];
			// Shift in 1's at top of shift register
			SPI_shift_reg[0] <= 1'b1;
		end

		if( ~SPI_LoadN )
		begin
			SPI_DataAck 	<= 1;
		end

		if( ~SPI_Data_RdyN )
		begin
			SPI_DataAck 	<= 0;
		end
	end
end

always @ (posedge SPI_Clk or negedge SPI_ResetN )
begin
	if(~SPI_ResetN) begin
		SPI_counter <= 4'b0000;
	end
	
	else begin
		// Counting each SCLK ensures we output in blocks
		// of 8 bits, as per SPI.
		SPI_counter[2:0]	<= SPI_counter[3:1];
		SPI_counter[3]	<= ~SPI_counter[0];
	end
end

endmodule	// SPI_SHIFT_OUT
