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

module SPI_SHIFT_OUT( 	SPI_Clk,
						SPI_ResetN,
						SPI_Data_RdyN,
						SPI_Data_In,
						SPI_Sdo,
						SPI_AlmostEmptyN,
						SPI_EmptyN
					);

// Inputs
input					SPI_Clk;
input					SPI_ResetN;
input					SPI_Data_RdyN;
input [(`SDEPTH-1):0]	SPI_Data_In;

//Outputs
output 					SPI_Sdo;
output 					SPI_AlmostEmptyN;
output 					SPI_EmptyN;

// Combinational Logic
reg [(`SDEPTH-1):0]		SPI_shift_reg;
reg [(`CWIDTH-1):0]		SPI_counter;

reg						SPI_DataRdy;

wire			SPI_Sdo	= SPI_shift_reg[(`SDEPTH-1)];

assign		SPI_AlmostEmptyN	= SPI_counter[3] | SPI_counter[2] | SPI_counter[1] | SPI_counter[0];
assign		SPI_EmptyN	= ~SPI_counter[3] | SPI_counter[2] | SPI_counter[1] | SPI_counter[0];

always @ (negedge SPI_Clk or negedge SPI_ResetN )
begin
	if(~SPI_ResetN) begin
		SPI_shift_reg	<= 8'b11111111;
	end
	
	else begin
		if (SPI_DataRdy)
		begin
			// If there is data ready and the shift register is empty,
			// do a load
			SPI_shift_reg[(`SDEPTH-1):0] <= SPI_Data_In[(`SDEPTH-1):0];
		end
		else
		// if ( SPI_Data_RdyN || SPI_EmptyN )
		begin
			// Shift bitstream_in into shift[0]
			SPI_shift_reg[(`SDEPTH-1):1] <= SPI_shift_reg[(`SDEPTH-2):0];
			// Shift in 1's at top of shift register
			SPI_shift_reg[0] <= 1'b1;
		end
	end
end

always @ (posedge SPI_Clk or negedge SPI_ResetN )
begin
	if(~SPI_ResetN) begin
		SPI_counter <= 4'b1000;
	end
	
	else begin
		// Decrementing on every shift ensures we output in blocks
		// of 8 bits, as per SPI. The SPI_EmptyN is active only
		// when SPI_counter equals 0
		SPI_counter[2:0]	<= SPI_counter[3:1];
		SPI_counter[3]	<= ~SPI_counter[0];

	end
end

always @ (posedge SPI_Clk or negedge SPI_ResetN )
begin
	if(~SPI_ResetN) begin
		SPI_DataRdy <= 0;
	end
	
	else begin
		SPI_DataRdy <= ~SPI_Data_RdyN;
	end
end

endmodule	// SPI_SHIFT_OUT
