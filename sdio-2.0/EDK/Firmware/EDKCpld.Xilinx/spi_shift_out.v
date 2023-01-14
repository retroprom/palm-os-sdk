/////////////////////////////////////////////////////////////////////////////
//
// SDIO CPLD SPI_SHIFT_OUT.v Src File
//
// Copyright (c) 2001 Palm Inc. or its subsidiaries.
// All rights reserved.  This software may be copied and used solely for 
// developing products for the communicating with the Palm Inc. platform
// and for archival and backup purposes.  Except for the foregoing, no part 
// of this software may be reproduced or transmitted in any form or by any 
// means or used to make any derivative work (such as translation, 
// transformation or adaptation) without express written consent from Palm Inc.
//
// Palm Inc. reserves the right to revise this software and to make changes in 
// content from time to time without obligation on the part of Palm Inc. to 
// provide notification of such revision or changes.  PALM INC. MAKES NO 
// REPRESENTATIONS OR WARRANTIES THAT THE SOFTWARE IS FREE OF ERRORS OR 
// THAT THE SOFTWARE IS SUITABLE FOR YOUR USE.  THE SOFTWARE IS PROVIDED ON
// AN "AS IS" BASIS.  PALM INC. MAKES NO WARRANTIES, TERMS OR CONDITIONS, EXPRESS
// OR IMPLIED, EITHER IN FACT OR BY OPERATION OF LAW, STATUTORY OR 
// OTHERWISE, INCLUDING WARRANTIES, TERMS, OR CONDITIONS OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, AND SATISFACTORY QUALITY.
//
// TO THE FULL EXTENT ALLOWED BY LAW, PALM INC. ALSO EXCLUDES FOR ITSELF AND ITS
// SUPPLIERS ANY LIABILITY, WHETHER BASED IN CONTRACT OR TORT (INCLUDING 
// NEGLIGENCE), FOR DIRECT, INCIDENTAL, CONSEQUENTIAL, INDIRECT, SPECIAL, OR
// PUNITIVE DAMAGES OF ANY KIND, OR FOR LOSS OF REVENUE OR PROFITS, LOSS OF
// BUSINESS, LOSS OF INFORMATION OR DATA, OR OTHER FINANCIAL LOSS ARISING 
// OUT OF OR IN CONNECTION WITH THIS SOFTWARE, EVEN IF PALM INC. HAS BEEN ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGES.
//
// HotSync, Palm Inc., and Graffiti are registered trademarks, 
// and Palm III and Palm OS are trademarks of Palm Inc. or its 
// subsidiaries.
//
// IF THIS SOFTWARE IS PROVIDED ON A COMPACT DISK, THE OTHER SOFTWARE AND 
// DOCUMENTATION ON THE COMPACT DISK ARE SUBJECT TO THE LICENSE AGREEMENT 
// ACCOMPANYING THE COMPACT DISK.
// 
/////////////////////////////////////////////////////////////////////////////
//
// Revision History:
//
// Date         Name        Description
// ----------------------------------------------------------
// 12/20/01		GAR         initial release
//
/////////////////////////////////////////////////////////////////////////////
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
			SPI_DataAck 	<= SPI_Data_RdyN;
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
